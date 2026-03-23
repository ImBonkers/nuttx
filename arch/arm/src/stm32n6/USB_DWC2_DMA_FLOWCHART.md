# USB DWC2 Buffer DMA Mode — EP0 Control Transfer Flowcharts

Comparison of ST HAL, Zephyr, and NuttX implementations for STM32N6
(DWC2 v5.00, CID 0x5000).

## Table of Contents
- [ST HAL Flows](#st-hal-stm32n6---buffer-dma-ep0-flow)
- [Zephyr Flows](#zephyr-dwc2-buffer-dma---ep0-flow)
- [Critical Differences](#critical-differences-affecting-nuttx)
- [Register Write Policies](#register-write-policies)
- [Sources](#sources)

---

## ST HAL (STM32N6) — Buffer DMA EP0 Flow

### PHY + Core Init

```
HAL_PCD_Init(hpcd)
  |
  +-> __HAL_PCD_DISABLE(hpcd)          // GAHBCFG &= ~GINT (disable global IRQ)
  |
  +-> USB_CoreInit(USBx, cfg)
  |     |
  |     +-> GUSBCFG &= ~TSDPS          // select data-line pulsing
  |     +-> USB_CoreReset(USBx)         // wait AHBIDL, set CSRST, wait clear
  |     +-> [if dma_enable == 1]
  |           GDFIFOCFG: reserve 18 FIFO locations (upper 16 bits = 0x3EE)
  |           GAHBCFG &= ~HBSTLEN; GAHBCFG |= HBSTLEN_INCR4
  |           GAHBCFG |= DMAEN
  |
  +-> USB_SetCurrentMode(USBx, USB_DEVICE_MODE)
  |     |
  |     +-> GUSBCFG &= ~(FHMOD | FDMOD)
  |     +-> GUSBCFG |= FDMOD
  |     +-> poll until GINTSTS.mode == Device (up to 200ms)
  |
  +-> USB_DevInit(USBx, cfg)
  |     |
  |     +-> Clear DIEPTXF[0..14] = 0
  |     +-> GCCFG &= ~PULLDOWNEN
  |     +-> [if vbus_sensing == 0]
  |     |     DCTL |= SDIS
  |     |     GCCFG |= VBVALEXTOEN | VBVALOVAL
  |     +-> PCGCCTL = 0 (restart PHY clock)
  |     +-> DCFG: set DEVSPD (HS or HS-in-FS)
  |     +-> FlushTxFifo(0x10), FlushRxFifo
  |     +-> DIEPMSK=0, DOEPMSK=0, DAINTMSK=0
  |     +-> For each EP: SNAK, clear DIEPTSIZ/DOEPTSIZ, clear DOEPINT=0xFB7F
  |     +-> [if dma_enable == 0] GINTMSK |= RXFLVLM    // NOT set in DMA mode!
  |     +-> GINTMSK |= USBSUSPM|USBRST|ENUMDNEM|IEPINT|OEPINT|IISOIXFRM|PXFRM|WUIM
  |
  +-> USB_DevDisconnect()               // DCTL |= SDIS
```

**Key DMA distinction**: In DMA mode, `RXFLVLM` is NOT enabled in GINTMSK.
The RxFIFO level interrupt is only for slave/completer mode.

---

### USB Reset Handler (GINTSTS.USBRST)

```
HAL_PCD_IRQHandler -> GINTSTS.USBRST set
  |
  +-> DCTL &= ~RWUSIG
  +-> FlushTxFifo(0x10)                // flush all TX FIFOs
  +-> For each EP (0..dev_endpoints):
  |     DIEPINT = 0xFB7F               // clear all IN EP interrupts
  |     DIEPCTL &= ~STALL
  |     DOEPINT = 0xFB7F               // clear all OUT EP interrupts
  |     DOEPCTL &= ~STALL
  |     DOEPCTL |= SNAK                // set NAK on all OUT EPs
  |
  +-> DAINTMSK |= 0x10001             // unmask EP0 IN + EP0 OUT
  +-> DOEPMSK |= STUPM|XFRCM|EPDM|OTEPSPRM|NAKM
  +-> DIEPMSK |= TOM|XFRCM|EPDM
  |
  +-> DCFG &= ~DAD                    // reset device address to 0
  +-> USB_EP0_OutStart(dma=1, psetup=hpcd->Setup)
  |     |
  |     +-> [CID > 0x300A] if DOEPCTL0.EPENA: return (skip!)
  |     +-> DOEPTSIZ0 = PKTCNT(1) | XFRSIZ(24) | STUPCNT(3)
  |     +-> [DMA] DOEPDMA0 = (uint32_t)psetup
  |     +-> [DMA] DOEPCTL0 |= EPENA | USBAEP      // *** NO CNAK ***
  |
  +-> Clear GINTSTS.USBRST
```

**Critical: `USB_EP0_OutStart` writes `EPENA|USBAEP` but NEVER `CNAK`.**

---

### Enumeration Done (GINTSTS.ENUMDNE)

```
HAL_PCD_IRQHandler -> GINTSTS.ENUMDNE set
  |
  +-> USB_ActivateSetup(USBx)
  |     +-> DIEPCTL0 &= ~MPSIZ         // set IN EP0 MPS to 64 bytes
  |     +-> DCTL |= CGINAK             // clear global IN NAK
  |
  +-> hpcd->Init.speed = USB_GetDevSpeed()   // read DSTS.ENUMSPD
  +-> USB_SetTurnaroundTime(hclk, speed)     // set GUSBCFG.TRDT
  +-> HAL_PCD_ResetCallback(hpcd)            // user callback
  +-> Clear GINTSTS.ENUMDNE
```

---

### SETUP Reception (DMA Mode)

```
Host sends SETUP packet
  |
  v
DWC2 core DMAs 8-byte setup data to address in DOEPDMA0
DWC2 core sets DOEPINT0: XFRC + STUP + STPKTRX (on CID > 0x300A)
  |
  v
HAL_PCD_IRQHandler -> GINTSTS.OEPINT
  |
  +-> ep_intr = DAINT & DAINTMSK (OUT EPs)
  +-> [EP0 bit set]
       |
       +-> epint = DOEPINT0 & DOEPMSK        // masked interrupt bits
       |
       +-> [XFRC set?] ──────────── YES ─────────────────────────────┐
       |     Clear DOEPINT0.XFRC                                     |
       |     PCD_EP_OutXfrComplete_int(hpcd, 0):                     |
       |       |                                                     |
       |       +-> DoepintReg = DOEPINT0 (raw, unmasked)             |
       |       |                                                     |
       |       +-> CASE A: [STUP set?] ──► "Class C"                |
       |       |     [CID > 0x300A && STPKTRX?] clear STPKTRX       |
       |       |     return (STUP handler will dispatch)             |
       |       |                                                     |
       |       +-> CASE B: [OTEPSPR set?] ──► "Status Phase Rcvd"   |
       |       |     clear OTEPSPR                                   |
       |       |     return (no data callback)                       |
       |       |                                                     |
       |       +-> CASE C: [neither STUP nor OTEPSPR]               |
       |             [CID > 0x300A && STPKTRX?] clear, return       |
       |             [else: genuine data completion]                  |
       |               xfer_count = xfer_size - DOEPTSIZ0.XFRSIZ    |
       |               [EP0 ZLP?] USB_EP0_OutStart (re-arm)          |
       |               HAL_PCD_DataOutStageCallback(hpcd, 0)         |
       |                                                             |
       +-> [STUP set?] ──────────── YES ─────────────────────────────┘
             Clear DOEPINT0.STUP
             PCD_EP_OutSetupPacket_int(hpcd, 0):
               |
               +-> [CID > 0x300A && STPKTRX?] clear STPKTRX
               +-> HAL_PCD_SetupStageCallback(hpcd)  ◄── DISPATCH
               +-> [CID > 0x300A && DMA]
                     USB_EP0_OutStart(1, hpcd->Setup) ◄── RE-ARM
```

**Processing order**: XFRC is checked FIRST, then STUP. When XFRC fires
alongside STUP, the XFRC handler detects this and bails out. The actual
SETUP dispatch happens in the STUP handler.

---

### Control-IN Transfer (GET_DESCRIPTOR)

```
  App calls HAL_PCD_EP_Transmit(EP0_IN, buf, len)
    +-> USB_EPStartXfer:
    |   DIEPTSIZ0 = PKTCNT|XFRSIZ(len)
    |   DIEPDMA0 = buf
    |   DIEPCTL0 |= CNAK|EPENA
    |
    v
  DWC2 sends data -> host ACKs -> DIEPINT0.XFRC
    |
    v
  EP0 IN XFRC handler:
    +-> [DMA, EP0, xfer_len==0 (ZLP just sent)]
    |     USB_EP0_OutStart(setup_buf) ◄── re-arm for next SETUP
    +-> HAL_PCD_DataInStageCallback(hpcd, 0)
    |
    v
  Host sends ZLP OUT (status stage):
    +-> DOEPINT0: XFRC + OTEPSPR
    +-> XFRC handler sees OTEPSPR -> clear, bail (no data callback)
    +-> EP0 OUT stays armed for next SETUP
```

**Key**: After IN ZLP completes on EP0, HAL calls `USB_EP0_OutStart` to
re-arm. The ZLP OUT status stage is handled via OTEPSPR (clear and ignore).

---

### Control-OUT Transfer (SET_LINE_CODING)

```
  1. SETUP arrives (SET_LINE_CODING, wLength=7)
     -> SetupStageCallback dispatches to class driver
     -> Class calls HAL_PCD_EP_Receive(EP0_OUT, buf, 7)
          +-> USB_EPStartXfer:
              DOEPTSIZ0 = PKTCNT(1)|XFRSIZ(64)
              DOEPDMA0 = buf
              DOEPCTL0 |= CNAK|EPENA    ◄── CNAK written for data phase!

  2. Host sends 7 bytes -> DWC2 DMAs -> DOEPINT0.XFRC
     +-> [no STUP, no OTEPSPR, no STPKTRX]
     +-> xfer_count = xfer_size - DOEPTSIZ0.XFRSIZ
     +-> HAL_PCD_DataOutStageCallback(hpcd, 0)

  3. Class sends ZLP IN (status):
     HAL_PCD_EP_Transmit(EP0_IN, NULL, 0)
       DIEPTSIZ0 = PKTCNT(1)|XFRSIZ(0)
       DIEPCTL0 |= CNAK|EPENA

  4. EP0 IN XFRC (ZLP sent):
     -> USB_EP0_OutStart(setup_buf) ◄── re-arm for next SETUP
```

---

### USB_EP0_OutStart — The DOEPCTL Write Policy

```
USB_EP0_OutStart(USBx, dma, psetup):

  GUARD: [CID > 0x300A]
    if (DOEPCTL0 & EPENA) == EPENA:
      return HAL_OK                    ◄── SKIP if already enabled

  DOEPTSIZ0 = 0
  DOEPTSIZ0 |= PKTCNT(1)              // 1 packet
  DOEPTSIZ0 |= XFRSIZ(24)             // 3 x 8 = 24 bytes
  DOEPTSIZ0 |= STUPCNT(3)             // bits [30:29] = 3

  [DMA]:
    DOEPDMA0 = (uint32_t)psetup       // setup buffer address
    DOEPCTL0 |= EPENA | USBAEP        ◄── NO CNAK!

  CNAK is NEVER written in USB_EP0_OutStart.
  CNAK is only written in USB_EPStartXfer (explicit data transfers).
```

**When called**:
1. USB Reset handler (after clearing DAD)
2. After STUP interrupt processed (CID > 0x300A + DMA)
3. After EP0 IN ZLP completes (status stage of control-IN)
4. After EP0 OUT ZLP completes (status stage of control-OUT)
5. After EP0 stall

---

## Zephyr DWC2 Buffer DMA — EP0 Flow

### USB Reset (dwc2_on_bus_reset)

```
dwc2_on_bus_reset(dev):
  |
  +-> For each OUT EP: write SNAK to DOEPCTLn
  +-> DOEPMSK = SETUP | XFERCOMPL | EPDISBLD | STSPHSERCVD  ◄── KEY!
  +-> DIEPMSK = INEPNAKEFF | EPDISBLD | XFERCOMPL
  +-> [completer mode only] GINTMSK |= RXFLVL
  +-> DCFG &= ~DEVADDR (address = 0)
```

**Key**: STSPHSERCVD is enabled in DOEPMSK specifically for buffer DMA mode.
RXFLVL is NOT enabled in DMA mode.

---

### SETUP Reception (Buffer DMA)

```
DWC2 DMAs 8 bytes to buffer, advances DOEPDMA by 8
Sets DOEPINT0: XFERCOMPL + STUPPKTRCVD

dwc2_handle_oepint():
  status = DOEPINT0 & DOEPMSK
  Write status -> DOEPINT0 (clear handled bits)

  ┌─ [DMA && XFERCOMPL && raw STUPPKTRCVD] ──────────┐
  │  Clear STUPPKTRCVD in DOEPINT0                    │
  │  status &= ~XFERCOMPL  (suppress XFRC processing) │
  │  addr = read DOEPDMA0  (points PAST setup data)   │
  │  cache_invalidate(addr - 8, 8)                    │
  │  memcpy(priv->setup, addr - 8, 8)  ◄── KEY!      │
  └───────────────────────────────────────────────────┘

  [status & STSPHSERCVD]:
    dwc2_clear_control_in_nak(dev) ◄── CRITICAL!
      read DIEPCTL0
      if NAKSTS: write (DIEPCTL0 & ~EPENA) | CNAK

  [status & SETUP]:
    post DWC2_DRV_EVT_SETUP -> thread handles dispatch

  [status & XFERCOMPL]:  (only if not suppressed above)
    dwc2_handle_out_xfercompl(dev, n)
```

**STUPPKTRCVD handling**: In buffer DMA, when both XFERCOMPL and
STUPPKTRCVD are set, Zephyr suppresses XFRC and reads SETUP data from
`DOEPDMA - 8` (the DMA pointer advanced past it).

---

### EP0 Control Transfer State Machine

```
Zephyr uses buffer flags (bi->setup, bi->data, bi->status)

SETUP Stage:
  [DWC2_DRV_EVT_SETUP in thread]
  -> dwc2_handle_evt_setup(dev)
       udc_dwc2_ep_disable(dev, EP0_IN, stall=false, wait=true)
       udc_setup_received(dev, priv->setup)  // dispatch to USB stack

DATA Stage (Control-OUT, e.g., SET_LINE_CODING):
  USB stack queues buf with bi->data=1 on EP0 OUT
  -> dwc2_prep_rx(dev, buf, cfg):
       DOEPTSIZ0 = PKTCNT(1)|XFRSIZ(tailroom)|SUPCNT(3)
       DOEPDMA0 = net_buf_tail(buf)
       DOEPCTL0 |= EPENA | CNAK    ◄── CNAK for data phase

DATA Stage (Control-IN, e.g., GET_DESCRIPTOR):
  USB stack queues buf with bi->data=1 on EP0 IN
  -> dwc2_clear_control_in_nak(dev)   // clear pending NAK
  -> dwc2_tx_fifo_write(dev, cfg, buf):
       DIEPTSIZ0 = PKTCNT(pktcnt)|XFRSIZ(len)
       DIEPDMA0 = buf->data
       DIEPCTL0 |= EPENA             ◄── NO CNAK for EP0 IN

STATUS Stage (OUT ZLP after Control-IN):
  USB stack queues buf with bi->status=1 on EP0 OUT
  -> dwc2_prep_rx(dev, buf, cfg):
       DOEPTSIZ0 = PKTCNT(1)|XFRSIZ(tailroom)|SUPCNT(3)
       DOEPDMA0 = net_buf_tail(buf)
       DOEPCTL0 |= EPENA | CNAK    ◄── CNAK for status phase

STATUS Stage (IN ZLP after Control-OUT):
  USB stack queues buf with bi->status=1 on EP0 IN
  -> dwc2_clear_control_in_nak(dev)
  -> dwc2_tx_fifo_write(dev, cfg, buf):
       DIEPTSIZ0 = PKTCNT(1)|XFRSIZ(0)  // ZLP
       DIEPCTL0 |= EPENA
```

---

### CNAK Policy Summary

```
EP0 OUT arming (dwc2_prep_rx):
  DOEPTSIZ0 = PKTCNT|XFRSIZ|SUPCNT(3)
  DOEPDMA0 = buffer
  DOEPCTL0 |= EPENA
  if (bi->data || bi->status):
    DOEPCTL0 |= CNAK     ◄── ONLY for data/status OUT phases
  // SETUP re-arm: NO CNAK ◄── prevents DMA controller lockup

EP0 IN (dwc2_tx_fifo_write):
  DIEPTSIZ0 = PKTCNT|XFRSIZ
  DIEPDMA0 = buffer
  DIEPCTL0 |= EPENA      ◄── NO CNAK
  // CNAK done separately via dwc2_clear_control_in_nak()

Non-EP0 endpoints:
  CNAK always written in both dwc2_prep_rx and dwc2_tx_fifo_write
```

---

### STSPHSERCVD Handling

```
dwc2_handle_oepint():
  [status & STSPHSERCVD]:
    dwc2_clear_control_in_nak(dev)
      |
      +-> diepctl = read DIEPCTL0
      +-> [if NAKSTS set]:
            diepctl &= ~EPENA
            diepctl |= CNAK
            write DIEPCTL0
```

**Purpose**: After a control-IN data stage, the host sends OUT ZLP (status).
DWC2 signals this with STSPHSERCVD. The driver must clear IN NAK so
subsequent SETUP IN responses can proceed. **Without this, SETUP interrupts
cease after the first device-to-host control transfer in buffer DMA mode.**

---

## Critical Differences Affecting NuttX

| Aspect | ST HAL | Zephyr | NuttX (current) |
|---|---|---|---|
| **CNAK in SETUP re-arm** | NEVER | NEVER | **REMOVED** (was YES) |
| **STSPHSERCVD in DOEPMSK** | Yes (OTEPSPRM) | Yes (DMA only) | **ADDED** |
| **STSPHSERCVD handler** | Clear only | Clear + clear IN NAK | **ADDED** |
| **RXFLVL in DMA mode** | DISABLED | DISABLED | **ENABLED** (mismatch) |
| **STUPPKTRCVD in XFRC** | Check + suppress data | Check + suppress XFRC + read DMA-8 | Check + suppress XFRC |
| **EPENA guard in ctrlsetup** | Skip entire write if EPENA set | No explicit guard (event-driven) | Skip entire write if EPENA set |
| **EP0 IN CNAK** | Written in EPStartXfer | Never in DIEPCTL; separate clear_in_nak | Written in epin_transfer |
| **Setup data source (DMA)** | Pre-programmed buffer (hpcd->Setup) | DOEPDMA - 8 with cache invalidate | Pre-programmed buffer (setup_buf) |
| **IRQ vs thread dispatch** | All in ISR | ISR posts events, thread dispatches | All in ISR |

---

## Register Write Policies

### USB_EP0_OutStart / ctrlsetup

| | DOEPTSIZ0 | DOEPDMA0 | DOEPCTL0 |
|---|---|---|---|
| **ST HAL** | PKTCNT(1)\|XFRSIZ(24)\|STUPCNT(3) | setup_buf | EPENA\|USBAEP (NO CNAK) |
| **Zephyr** | PKTCNT(1)\|XFRSIZ(tailroom)\|SUPCNT(3) | buffer | EPENA only (NO CNAK) |
| **Guard** | Always written | Always written | Skip if EPENA already set (HAL) |

### Data OUT phase (EP_Receive / prep_rx for data)

| | DOEPTSIZ0 | DOEPDMA0 | DOEPCTL0 |
|---|---|---|---|
| **ST HAL** | PKTCNT(1)\|XFRSIZ(maxpacket) | data_buf | CNAK\|EPENA |
| **Zephyr** | PKTCNT(1)\|XFRSIZ(tailroom)\|SUPCNT(3) | data_buf | CNAK\|EPENA |

### Data IN phase (EP_Transmit / tx_fifo_write)

| | DIEPTSIZ0 | DIEPDMA0 | DIEPCTL0 |
|---|---|---|---|
| **ST HAL** | PKTCNT\|XFRSIZ(len) | data_buf | CNAK\|EPENA |
| **Zephyr** | PKTCNT\|XFRSIZ(len) | data_buf | EPENA only (CNAK via separate fn) |

---

## Sources

- **ST HAL**: `SampleSTM32Project/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_hal_pcd.c`
- **ST LL USB**: `SampleSTM32Project/Drivers/STM32N6xx_HAL_Driver/Src/stm32n6xx_ll_usb.c`
- **Zephyr**: `drivers/usb/udc/udc_dwc2.c` ([GitHub](https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/usb/udc/udc_dwc2.c))
- **NuttX STM32H7**: `arch/arm/src/stm32h7/stm32_otgdev.c` (PIO only, no DMA)
- **Linux DWC2**: `drivers/usb/dwc2/gadget.c`
- **DWC2 Databook**: Synopsys DesignWare Cores USB 2.0 Hi-Speed On-The-Go (OTG)
