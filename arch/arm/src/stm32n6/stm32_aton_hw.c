/****************************************************************************
 * arch/arm/src/stm32n6/stm32_aton_hw.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Native ATON NPU register programming for STM32N6.
 * Reimplements the 7 LL_ATON functions that STEdgeAI-generated model
 * code calls, using pure NuttX register access (putreg32/getreg32).
 * Replaces the entire LL_ATON runtime (~15K lines) with ~800 lines.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <nuttx/cache.h>

#include "stm32_aton.h"
#include "stm32_aton_hw.h"

/****************************************************************************
 * Private Inline Functions
 ****************************************************************************/

/* Direct register access — this file is compiled as app code so we
 * cannot use the kernel's putreg32/getreg32 from arm_internal.h.
 */

static inline void putreg32(uint32_t val, uintptr_t addr)
{
  *(volatile uint32_t *)addr = val;
}

static inline uint32_t getreg32(uintptr_t addr)
{
  return *(volatile uint32_t *)addr;
}

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ATON_MIN(a, b)  ((a) < (b) ? (a) : (b))

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Saved CONVACC CTRL values for EnableUnits (must preserve config bits) */

static uint32_t g_convacc_ctrl[ATON_CONVACC_NUM];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: aton_disable_clr_confclr
 *
 * Description:
 *   Disable + clear + config-clear a unit via its CTRL register.
 *   Sequence: CLR=1 → poll CLR==0 → CONFCLR=1 → poll CONFCLR==0
 *
 ****************************************************************************/

static void aton_disable_clr_confclr(uintptr_t ctrl_addr)
{
  /* Write clean CLR=1 (all other bits 0, matching original LL_ATON) */

  putreg32((1u << 1), ctrl_addr);

  /* Poll until CLR auto-clears */

  while (getreg32(ctrl_addr) & (1u << 1));

  /* Write clean CONFCLR=1 */

  putreg32((1u << 30), ctrl_addr);

  /* Poll until CONFCLR auto-clears */

  while (getreg32(ctrl_addr) & (1u << 30));
}

/****************************************************************************
 * Name: get_arith_hwop
 *
 * Description:
 *   Convert LL_Arithacc_Op enum to hardware operation code.
 *
 ****************************************************************************/

static uint32_t get_arith_hwop(LL_Arithacc_Op op)
{
  /* LL_ATON enum values map 1:1 minus 1 to HW codes for AFFINE through
   * CLIP.  AFFINE=1 → HW 0, MIN=2 → HW 1, etc.
   */

  if (op >= ARITH_AFFINE && op <= ARITH_CLIP)
    {
      return (uint32_t)(op - 1);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: LL_ATON_EnableClock
 *
 * Description:
 *   Enable a clock gate bit in CLKCTRL BGATES register.
 *
 ****************************************************************************/

void LL_ATON_EnableClock(unsigned int clock)
{
  uintptr_t addr = ATON_CLKCTRL_BASE + ATON_CLKCTRL_BGATES;
  uint32_t t;

  t = getreg32(addr);
  t |= (1u << clock);
  putreg32(t, addr);
}

/****************************************************************************
 * Name: LL_ATON_DisableClock
 *
 * Description:
 *   Disable a clock gate bit in CLKCTRL BGATES register.
 *
 ****************************************************************************/

void LL_ATON_DisableClock(unsigned int clock)
{
  uintptr_t addr = ATON_CLKCTRL_BASE + ATON_CLKCTRL_BGATES;
  uint32_t t;

  t = getreg32(addr);
  t &= ~(1u << clock);
  putreg32(t, addr);
}

/****************************************************************************
 * Name: LL_Streng_TensorInit
 *
 * Description:
 *   Program a streaming engine for data transfer.
 *
 ****************************************************************************/

int LL_Streng_TensorInit(int id,
                         const LL_Streng_TensorInitTypeDef *conf, int n)
{
  uintptr_t base;
  uint32_t ctrl;
  uint32_t strd;
  uint32_t cid_cache;
  uint32_t event;
  uint32_t t;
  int ch_bits[3] = {0, 0, 0};
  int nbits_in;
  int nbits_out;
  int io_case;
  int bits[3];
  unsigned line_offset;

  if (id >= ATON_STRENG_NUM || n != 1)
    {
      return -1;
    }

  base = ATON_STRENG_BASE(id);

  /* Enable clock gate */

  LL_ATON_EnableClock(ATON_CLKB_STRENG(id));

  /* Build CTRL register */

  ctrl = 0;
  if (conf->dir)
    {
      ctrl |= STRENG_CTRL_DIR;
    }

  if (conf->raw)
    {
      ctrl |= STRENG_CTRL_RAW;
    }

  if (conf->raw_out)
    {
      ctrl |= STRENG_CTRL_RAW_OUT;
    }

  if (conf->noblk)
    {
      ctrl |= STRENG_CTRL_NOBLK;
    }

  if (conf->noinc)
    {
      ctrl |= STRENG_CTRL_NOINC;
    }

  if (conf->frame_tot_cnt == 1)
    {
      ctrl |= STRENG_CTRL_SINGLE;
    }

  if (conf->continuous)
    {
      ctrl |= STRENG_CTRL_CONT;
    }

  if (conf->align_right)
    {
      ctrl |= STRENG_CTRL_LSBMODE;
      if (!conf->nbits_unsigned)
        {
          ctrl |= STRENG_CTRL_SIGNEXT;
        }
    }

  /* Compute SIZE0/SIZE1/SIZE2 channel bits */

  nbits_in  = conf->nbits_in;
  nbits_out = conf->nbits_out;
  strd = 0;

  io_case = ((conf->dir != 0) << 1) |
            (conf->dir == 0 ? (nbits_in <= nbits_out) :
                              (nbits_in < nbits_out));

  switch (io_case)
    {
      case 0: /* bus→stream, in > out */
        if (conf->mem_lsb)
          {
            strd |= ((nbits_in - nbits_out) << STRENG_STRD_FGAP_SHIFT)
                     & STRENG_STRD_FGAP_MASK;
          }
        else
          {
            strd |= ((nbits_in - nbits_out) << STRENG_STRD_BGAP_SHIFT)
                     & STRENG_STRD_BGAP_MASK;
          }

        nbits_in = nbits_out;

        /* Fall through */

      case 1: /* bus→stream, in <= out */
        bits[0] = ATON_MIN(8, nbits_in);
        bits[1] = nbits_in > 8  ? ATON_MIN(8, nbits_in - 8)  : 0;
        bits[2] = nbits_in > 16 ? ATON_MIN(8, nbits_in - 16) : 0;
        if (conf->align_right)
          {
            ch_bits[0] = bits[0];
            ch_bits[1] = bits[1];
            ch_bits[2] = bits[2];
          }
        else
          {
            if (nbits_out > 16)
              {
                ch_bits[2] = bits[0];
                ch_bits[1] = bits[1];
                ch_bits[0] = bits[2];
              }
            else if (nbits_out > 8)
              {
                ch_bits[1] = bits[0];
                ch_bits[0] = bits[1];
              }
            else
              {
                ch_bits[0] = bits[0];
              }
          }
        break;

      case 3: /* stream→bus, in < out */
        if (conf->mem_lsb)
          {
            strd |= ((nbits_out - nbits_in) << STRENG_STRD_FGAP_SHIFT)
                     & STRENG_STRD_FGAP_MASK;
          }
        else
          {
            strd |= ((nbits_out - nbits_in) << STRENG_STRD_BGAP_SHIFT)
                     & STRENG_STRD_BGAP_MASK;
          }

        nbits_out = nbits_in;

        /* Fall through */

      case 2: /* stream→bus, in >= out */
        bits[0] = ATON_MIN(8, nbits_out);
        bits[1] = nbits_out > 8  ? ATON_MIN(8, nbits_out - 8)  : 0;
        bits[2] = nbits_out > 16 ? ATON_MIN(8, nbits_out - 16) : 0;
        if (conf->align_right)
          {
            ch_bits[0] = bits[0];
            ch_bits[1] = bits[1];
            ch_bits[2] = bits[2];
          }
        else
          {
            if (conf->nbits_in > 16)
              {
                ch_bits[2] = bits[0];
                ch_bits[1] = bits[1];
                ch_bits[0] = bits[2];
              }
            else if (conf->nbits_in > 8)
              {
                ch_bits[1] = bits[0];
                ch_bits[0] = bits[1];
              }
            else
              {
                ch_bits[0] = bits[0];
              }
          }
        break;
    }

  ctrl |= (ch_bits[0] << STRENG_CTRL_SIZE0_SHIFT) & STRENG_CTRL_SIZE0_MASK;
  ctrl |= (ch_bits[1] << STRENG_CTRL_SIZE1_SHIFT) & STRENG_CTRL_SIZE1_MASK;
  ctrl |= (ch_bits[2] << STRENG_CTRL_SIZE2_SHIFT) & STRENG_CTRL_SIZE2_MASK;

  putreg32(ctrl, base + ATON_STRENG_CTRL);

  /* ADDR = base + offset_start */

  putreg32(conf->addr_base.i + conf->offset_start,
           base + ATON_STRENG_ADDR);

  /* FSIZE */

  cid_cache = 0;

  if (conf->raw)
    {
      if (conf->frame_count)
        {
          t = conf->frame_count;
        }
      else
        {
          unsigned pixel_bits = conf->dir == 0 ? conf->nbits_in
                                               : conf->nbits_out;
          t = (LL_Streng_len(conf) * 8) / pixel_bits;
        }

      putreg32(t, base + ATON_STRENG_FSIZE);
    }
  else
    {
      t = (conf->fwidth & 0xFFFF) |
          ((conf->fheight & 0xFFFF) << 16);
      putreg32(t, base + ATON_STRENG_FSIZE);

      line_offset = conf->line_offset == 0
                    ? conf->fwidth * conf->batch_offset
                    : conf->line_offset;
      strd |= (line_offset << STRENG_STRD_LOFF_SHIFT)
              & STRENG_STRD_LOFF_MASK;

      /* LOFF MSB in CID_CACHE for large line offsets */

      cid_cache |= ((line_offset >> 16) << STRENG_CID_LOFF_MSB_SHIFT)
                   & STRENG_CID_LOFF_MSB_MASK;

      /* DEPTH register */

      t = (conf->batch_depth & 0xFFFF) |
          ((conf->batch_offset & 0xFFFF) << 16);
      putreg32(t, base + ATON_STRENG_DEPTH);
    }

  /* Frame offset */

  putreg32(conf->frame_offset, base + ATON_STRENG_FOFFSET);

  /* Frame repeat */

  putreg32(conf->loop_offset, base + ATON_STRENG_FRPTOFF);
  putreg32(conf->frame_loop_cnt, base + ATON_STRENG_FRAME_RPT);

  /* LIMITEN */

  t = STRENG_LIMITEN_FRAMELIMIT;
  putreg32(t, base + ATON_STRENG_LIMITEN);

  if (conf->offset_limit != 0)
    {
      t |= STRENG_LIMITEN_ADDRLIMIT | STRENG_LIMITEN_STOPPREFTC;
      putreg32(t, base + ATON_STRENG_LIMITEN);
      putreg32(conf->addr_base.i + conf->offset_limit - 1,
               base + ATON_STRENG_LIMITADDR);
    }

  /* LIMIT (total frame count) */

  putreg32(conf->frame_tot_cnt, base + ATON_STRENG_LIMIT);

  /* CID_CACHE */

  cid_cache |= (conf->bus_cid << STRENG_CID_CID_SHIFT)
                & STRENG_CID_CID_MASK;
  if (conf->cacheable)
    {
      cid_cache |= STRENG_CID_CACHEABLE;
    }

  if (conf->cache_allocate)
    {
      cid_cache |= STRENG_CID_ALLOC;
    }

  if (conf->bus_pfetch)
    {
      cid_cache |= STRENG_CID_PFETCH;
    }

  cid_cache |= (conf->cache_linesize << STRENG_CID_LINESIZE_SHIFT)
                & STRENG_CID_LINESIZE_MASK;

  /* EVENT */

  event = 0;
  if (conf->dir == 1)
    {
      event |= STRENG_EVENT_EN_OFLOW_FRM;
    }

  event |= STRENG_EVENT_EN_ILLCFG;

  if (conf->dir == 0 && conf->sync_with_other)
    {
      event |= STRENG_EVENT_FRMTRG_EN;
      event |= (conf->sync_dma << STRENG_EVENT_FRMTRG_SRC_SHIFT)
               & STRENG_EVENT_FRMTRG_SRC_MASK;
    }

  /* POS (gap cycles = 0) */

  putreg32(0, base + ATON_STRENG_POS);

  /* Write deferred registers */

  putreg32(strd, base + ATON_STRENG_STRD);
  putreg32(cid_cache, base + ATON_STRENG_CID_CACHE);
  putreg32(event, base + ATON_STRENG_EVENT);

  return 0;
}

/****************************************************************************
 * Name: LL_Convacc_Init
 *
 * Description:
 *   Program a convolution accelerator unit.
 *
 ****************************************************************************/

int LL_Convacc_Init(int id, const LL_Convacc_InitTypeDef *conf)
{
  uintptr_t base;
  uint32_t ctrl;
  uint32_t t;
  int p_top;
  int p_bot;
  int p_left;
  int p_right;
  int z_top;
  int z_bot;
  int z_left;
  int z_right;

  if (id >= ATON_CONVACC_NUM)
    {
      return -1;
    }

  base = ATON_CONVACC_BASE(id);

  LL_ATON_EnableClock(ATON_CLKB_CONVACC(id));

  /* CTRL register */

  ctrl = 0;
  if (!conf->accumulate)
    {
      ctrl |= CONVACC_CTRL_NOSUM;
    }

  if (!conf->accumulate_first)
    {
      ctrl |= CONVACC_CTRL_NO1SUM;
    }

  if (conf->accumulate_gen_first)
    {
      ctrl |= CONVACC_CTRL_GEN1SUM;
    }

  ctrl |= (conf->afilt_mode << CONVACC_CTRL_AFILTMODE_SHIFT)
           & CONVACC_CTRL_AFILTMODE_MASK;
  ctrl |= (conf->simd << CONVACC_CTRL_SIMD_SHIFT)
           & CONVACC_CTRL_SIMD_MASK;

  if (conf->kt1_mode)
    {
      ctrl |= CONVACC_CTRL_KT1;
    }

  ctrl |= (conf->kseten << CONVACC_CTRL_KSETEN_SHIFT)
           & CONVACC_CTRL_KSETEN_MASK;

  if (conf->f_unsigned)
    {
      ctrl |= CONVACC_CTRL_FUNSIGNED;
    }

  if (conf->k_unsigned)
    {
      ctrl |= CONVACC_CTRL_KUNSIGNED;
    }

  if (conf->fstat)
    {
      ctrl |= CONVACC_CTRL_FSTAT;
    }

  if (conf->deepmode)
    {
      ctrl |= CONVACC_CTRL_DEEPMODE;
    }

  if (conf->dss2mode)
    {
      ctrl |= CONVACC_CTRL_DSS2MODE;
    }

  putreg32(ctrl, base + ATON_CONVACC_CTRL);
  g_convacc_ctrl[id] = ctrl;

  /* AFILT register */

  if (conf->afilt_mode != AFILT_MODE_NONE)
    {
      t = (conf->afilt_tot   << CONVACC_AFILT_TOT_SHIFT)   |
          (conf->afilt_first << CONVACC_AFILT_FIRST_SHIFT) |
          (conf->afilt_last  << CONVACC_AFILT_LAST_SHIFT);
    }
  else
    {
      t = 0x00000001;  /* AFILT_DT: TOT=1 (default) */
    }

  putreg32(t, base + ATON_CONVACC_AFILT);

  /* KFILT register */

  if (conf->kfilt_tot > 0)
    {
      t = (conf->kfilt_tot   << CONVACC_KFILT_TOT_SHIFT)   |
          (conf->kfilt_first << CONVACC_KFILT_FIRST_SHIFT) |
          (conf->kfilt_last  << CONVACC_KFILT_LAST_SHIFT);
    }
  else
    {
      t = 0x00000001;  /* KFILT_DT: TOT=1 (single kernel set) */
    }

  putreg32(t, base + ATON_CONVACC_KFILT);

  /* DFORMAT register */

  t = 0;
  t |= (conf->rounding_f  << CONVACC_DFORMAT_FROUND_SHIFT);
  t |= (conf->saturation_f << CONVACC_DFORMAT_FSAT_SHIFT);
  t |= (conf->round_mode_f << CONVACC_DFORMAT_FRNDMODE_SHIFT);
  t |= (conf->inbytes_f << CONVACC_DFORMAT_FBYTES_SHIFT)
       & CONVACC_DFORMAT_FBYTES_MASK;
  t |= (ATON_SHIFT_ENCODE(conf->shift_f) << CONVACC_DFORMAT_FSHIFT_SHIFT)
       & CONVACC_DFORMAT_FSHIFT_MASK;
  t |= (conf->raw_o << CONVACC_DFORMAT_RAW_SHIFT);
  t |= (conf->shift_a << CONVACC_DFORMAT_INSHIFT_SHIFT)
       & CONVACC_DFORMAT_INSHIFT_MASK;
  t |= (conf->rounding_o << CONVACC_DFORMAT_ROUND_SHIFT);
  t |= (conf->saturation_o << CONVACC_DFORMAT_SAT_SHIFT);
  t |= (((conf->relu_mode_o << 1) | conf->round_mode_o)
        << CONVACC_DFORMAT_ORNDMODE_SHIFT) & CONVACC_DFORMAT_ORNDMODE_MASK;
  t |= (conf->outbytes_o << CONVACC_DFORMAT_OBYTES_SHIFT)
       & CONVACC_DFORMAT_OBYTES_MASK;
  t |= (conf->shift_o << CONVACC_DFORMAT_OUTSHIFT_SHIFT);
  putreg32(t, base + ATON_CONVACC_DFORMAT);

  /* FFORMAT register (width x height) */

  t = ((conf->fWidth * conf->batchDepth) & 0xFFFF) |
      ((conf->fHeight & 0xFFFF) << 16);
  putreg32(t, base + ATON_CONVACC_FFORMAT);

  /* KFORMAT register */

  t = 0;
  t |= (conf->kernelWidth  << CONVACC_KFORMAT_WIDTH_SHIFT)
       & CONVACC_KFORMAT_WIDTH_MASK;
  t |= (conf->kernelHeight << CONVACC_KFORMAT_HEIGHT_SHIFT)
       & CONVACC_KFORMAT_HEIGHT_MASK;
  t |= (conf->batchDepth   << CONVACC_KFORMAT_BTCDEPTH_SHIFT)
       & CONVACC_KFORMAT_BTCDEPTH_MASK;
  t |= (conf->nKernels     << CONVACC_KFORMAT_NR_SHIFT)
       & CONVACC_KFORMAT_NR_MASK;
  putreg32(t, base + ATON_CONVACC_KFORMAT);

  /* Compute HW padding (capped at kernel-1 and max 2) */

  p_top   = ATON_MIN(ATON_MIN((int)conf->top_padding,
                              (int)conf->kernelHeight - 1), 2);
  p_bot   = ATON_MIN(ATON_MIN((int)conf->bot_padding,
                              (int)conf->kernelHeight - 1), 2);
  p_left  = ATON_MIN(ATON_MIN((int)conf->left_padding,
                              (int)conf->kernelWidth - 1), 2);
  p_right = ATON_MIN(ATON_MIN((int)conf->right_padding,
                              (int)conf->kernelWidth - 1), 2);

  /* deepmode/dss2mode/zfbias force HW padding to 0 */

  if (conf->deepmode || conf->dss2mode || conf->zfbias)
    {
      p_top = p_bot = p_left = p_right = 0;
    }

  /* Remaining padding → zero-frame */

  z_top   = conf->top_padding   - p_top;
  z_bot   = conf->bot_padding   - p_bot;
  z_left  = conf->left_padding  - p_left;
  z_right = conf->right_padding - p_right;

  /* ZFRAME register */

  t = ((z_top                      << CONVACC_ZFRAME_TOP_SHIFT)
                                    & CONVACC_ZFRAME_TOP_MASK)    |
      ((z_bot                      << CONVACC_ZFRAME_BOTTOM_SHIFT)
                                    & CONVACC_ZFRAME_BOTTOM_MASK) |
      (((z_left * conf->batchDepth) << CONVACC_ZFRAME_LEFT_SHIFT)
                                    & CONVACC_ZFRAME_LEFT_MASK)   |
      (((z_right * conf->batchDepth) << CONVACC_ZFRAME_RIGHT_SHIFT)
                                    & CONVACC_ZFRAME_RIGHT_MASK);
  putreg32(t, base + ATON_CONVACC_ZFRAME);

  /* SAMPLE register (strides + HW padding) */

  t = 0;
  t |= (p_top   << CONVACC_SAMPLE_TPAD_SHIFT) & CONVACC_SAMPLE_TPAD_MASK;
  t |= (p_bot   << CONVACC_SAMPLE_BPAD_SHIFT) & CONVACC_SAMPLE_BPAD_MASK;
  t |= (p_left  << CONVACC_SAMPLE_LPAD_SHIFT) & CONVACC_SAMPLE_LPAD_MASK;
  t |= (p_right << CONVACC_SAMPLE_RPAD_SHIFT) & CONVACC_SAMPLE_RPAD_MASK;
  t |= (conf->hstride << CONVACC_SAMPLE_HSTRD_SHIFT)
       & CONVACC_SAMPLE_HSTRD_MASK;
  t |= (conf->vstride << CONVACC_SAMPLE_VSTRD_SHIFT)
       & CONVACC_SAMPLE_VSTRD_MASK;
  if (conf->fstat)
    {
      t |= (conf->fstatcnt << CONVACC_SAMPLE_FSTATCNT_SHIFT)
           & CONVACC_SAMPLE_FSTATCNT_MASK;
    }

  putreg32(t, base + ATON_CONVACC_SAMPLE);

  /* FHCROP, FVCROP registers */

  t = 0;
  if (conf->left_crop > 0)
    {
      t |= ((conf->left_crop * conf->batchDepth)
            << CONVACC_FHCROP_LEFT_SHIFT) & CONVACC_FHCROP_LEFT_MASK;
    }

  if (conf->right_crop > 0)
    {
      t |= ((conf->right_crop * conf->batchDepth + conf->batchDepth - 1)
            << CONVACC_FHCROP_RIGHT_SHIFT) & CONVACC_FHCROP_RIGHT_MASK;
    }

  putreg32(t, base + ATON_CONVACC_FHCROP);

  t = 0;
  if (conf->top_crop > 0)
    {
      t |= (conf->top_crop << CONVACC_FVCROP_TOP_SHIFT)
           & CONVACC_FVCROP_TOP_MASK;
    }

  if (conf->bot_crop > 0)
    {
      t |= (conf->bot_crop << CONVACC_FVCROP_BOTTOM_SHIFT)
           & CONVACC_FVCROP_BOTTOM_MASK;
    }

  putreg32(t, base + ATON_CONVACC_FVCROP);

  /* FSUB + VSHIFT */

  if (conf->fsub != 0 || conf->vshift != 0)
    {
      t = ((conf->fsub << CONVACC_FSUB_FSUB_SHIFT)
           & CONVACC_FSUB_FSUB_MASK) |
          ((conf->vshift << CONVACC_FSUB_VSHIFT_SHIFT)
           & CONVACC_FSUB_VSHIFT_MASK);
      putreg32(t, base + ATON_CONVACC_FSUB);
    }

  /* ZFBIAS */

  if (conf->zfbias != 0)
    {
      t = (conf->zfbias << CONVACC_ZFBIAS_ZFBIAS_SHIFT)
          & CONVACC_ZFBIAS_ZFBIAS_MASK;
      putreg32(t, base + ATON_CONVACC_ZFBIAS);
    }

  return 0;
}

/****************************************************************************
 * Name: LL_Arithacc_Init
 *
 * Description:
 *   Program an arithmetic accelerator unit.
 *   Currently supports scalar mode only (all current model epochs use it).
 *
 ****************************************************************************/

int LL_Arithacc_Init(int id, const LL_Arithacc_InitTypeDef *conf)
{
  uintptr_t base;
  uint32_t ctrl;
  uint32_t t;

  if (id >= ATON_ARITH_NUM)
    {
      return -1;
    }

  base = ATON_ARITH_BASE(id);

  LL_ATON_EnableClock(ATON_CLKB_ARITH(id));

  /* Reset CTRL */

  putreg32(0, base + ATON_ARITH_CTRL);

  /* Build CTRL register */

  ctrl = 0;

  /* Counter enables: scalar mode → counters off */

  if (!conf->scalar)
    {
      ctrl |= ARITH_CTRL_CNT1;
      if (conf->bcast == ARITH_BCAST_CHAN)
        {
          ctrl |= ARITH_CTRL_CNT2 | ARITH_CTRL_CNT3;
        }
    }

  if (conf->rounding_o)
    {
      ctrl |= ARITH_CTRL_ROUND;
    }

  if (conf->saturation_o)
    {
      ctrl |= ARITH_CTRL_SAT;
    }

  ctrl |= (((conf->relu_mode_o << 1) | conf->round_mode_o)
           << ARITH_CTRL_ORNDMODE_SHIFT) & ARITH_CTRL_ORNDMODE_MASK;
  ctrl |= (conf->outbytes_o << ARITH_CTRL_OBYTES_SHIFT)
          & ARITH_CTRL_OBYTES_MASK;

  /* Coefficient types: scalar=0 means vector (COEFFA/B/C=1) */

  if (!conf->scalar)
    {
      ctrl |= ARITH_CTRL_COEFFA | ARITH_CTRL_COEFFB | ARITH_CTRL_COEFFC;
    }

  if (conf->dualinput)
    {
      ctrl |= ARITH_CTRL_DUALIN;
    }

  if (conf->combinebc)
    {
      ctrl |= ARITH_CTRL_COMBINEBC;
    }

  if (conf->clipout)
    {
      ctrl |= ARITH_CTRL_CLIPOUT;
    }

  ctrl |= (get_arith_hwop(conf->operation) << ARITH_CTRL_OP_SHIFT)
          & ARITH_CTRL_OP_MASK;

  if (conf->operation == ARITH_NOT_X)
    {
      ctrl |= ARITH_CTRL_LOGICALOP;
    }

  putreg32(ctrl, base + ATON_ARITH_CTRL);

  /* INSHIFTER register */

  t = 0;
  t |= (conf->inbytes_x << ARITH_INSHIFTER_FBYTESX_SHIFT)
       & ARITH_INSHIFTER_FBYTESX_MASK;
  t |= (ATON_SHIFT_ENCODE(conf->shift_x) << ARITH_INSHIFTER_FSHIFTX_SHIFT)
       & ARITH_INSHIFTER_FSHIFTX_MASK;
  t |= (conf->rounding_x  << ARITH_INSHIFTER_FROUNDX_SHIFT);
  t |= (conf->saturation_x << ARITH_INSHIFTER_FSATX_SHIFT);
  t |= (conf->round_mode_x << ARITH_INSHIFTER_FRNDMODEX_SHIFT)
       & ARITH_INSHIFTER_FRNDMODEX_MASK;
  t |= (conf->outbytes_x << ARITH_INSHIFTER_FOBYTESX_SHIFT)
       & ARITH_INSHIFTER_FOBYTESX_MASK;
  t |= (conf->inbytes_y << ARITH_INSHIFTER_FBYTESY_SHIFT)
       & ARITH_INSHIFTER_FBYTESY_MASK;
  t |= (ATON_SHIFT_ENCODE(conf->shift_y) << ARITH_INSHIFTER_FSHIFTY_SHIFT)
       & ARITH_INSHIFTER_FSHIFTY_MASK;
  t |= (conf->rounding_y  << ARITH_INSHIFTER_FROUNDY_SHIFT);
  t |= (conf->saturation_y << ARITH_INSHIFTER_FSATY_SHIFT);
  t |= (conf->round_mode_y << ARITH_INSHIFTER_FRNDMODEY_SHIFT)
       & ARITH_INSHIFTER_FRNDMODEY_MASK;
  t |= (conf->outbytes_y << ARITH_INSHIFTER_FOBYTESY_SHIFT)
       & ARITH_INSHIFTER_FOBYTESY_MASK;
  putreg32(t, base + ATON_ARITH_INSHIFTER);

  /* SHIFT register — V1 hardware (no V3 shift extension offset) */

  t = 0;
  t |= (conf->Ax_shift << ARITH_SHIFT_AX_SHIFT)
       & ARITH_SHIFT_AX_MASK;
  t |= (conf->By_shift << ARITH_SHIFT_BY_SHIFT)
       & ARITH_SHIFT_BY_MASK;
  t |= (conf->C_shift << ARITH_SHIFT_C_SHIFT)
       & ARITH_SHIFT_C_MASK;
  t |= (conf->shift_o << ARITH_SHIFT_RES_SHIFT)
       & ARITH_SHIFT_RES_MASK;
  putreg32(t, base + ATON_ARITH_SHIFT);

  /* COEFFAC register (scalar A and C) */

  t = ((uint32_t)(uint16_t)conf->A_scalar << ARITH_COEFFAC_A_SHIFT) |
      ((uint32_t)(uint16_t)conf->C_scalar << ARITH_COEFFAC_C_SHIFT);
  putreg32(t, base + ATON_ARITH_COEFFAC);

  /* COEFFB register (scalar B) */

  t = (uint32_t)(uint16_t)conf->B_scalar;
  putreg32(t, base + ATON_ARITH_COEFFB);

  /* CLIPRANGE (if enabled) */

  if (conf->clipout)
    {
      t = ((uint32_t)(uint16_t)conf->clipmin
           << ARITH_CLIPRANGE_CLIPMIN_SHIFT) |
          ((uint32_t)(uint16_t)conf->clipmax
           << ARITH_CLIPRANGE_CLIPMAX_SHIFT);
      putreg32(t, base + ATON_ARITH_CLIPRANGE);
    }

  /* Reset coefficient address */

  putreg32(0, base + ATON_ARITH_COEFFADDR);

  return 0;
}

/****************************************************************************
 * Name: LL_Switch_Init
 *
 * Description:
 *   Program STRSWITCH destination routing.
 *   First resets the switch, then programs DST registers.
 *
 ****************************************************************************/

int LL_Switch_Init(const LL_Switch_InitTypeDef *sw, int n)
{
  int i;
  uint32_t t;
  uintptr_t reg;

  /* Reset the switch: disable+clr+confclr */

  aton_disable_clr_confclr(ATON_STRSWITCH_BASE + ATON_STRSWITCH_CTRL);

  /* Enable switch */

  putreg32(ATON_STRSWITCH_CTRL_EN,
           ATON_STRSWITCH_BASE + ATON_STRSWITCH_CTRL);

  /* Program each routing entry */

  for (i = 0; i < n; i++)
    {
      reg = ATON_STRSWITCH_BASE + ATONN_DSTPORT_ID(sw[i].dest);

      t = 0;

      /* Context 0 */

      t |= ((sw[i].context0 != 0) << ATON_DST_EN0_SHIFT);
      t |= (ATONN_SRCPORT_ID(sw[i].source0) << ATON_DST_LINK0_SHIFT)
           & ATON_DST_LINK0_MASK;
      t |= ((uint32_t)sw[i].frames0 << ATON_DST_FNR0_SHIFT)
           & ATON_DST_FNR0_MASK;

      /* Context 1 */

      t |= ((sw[i].context1 != 0) << ATON_DST_EN1_SHIFT);
      t |= (ATONN_SRCPORT_ID(sw[i].source1) << ATON_DST_LINK1_SHIFT)
           & ATON_DST_LINK1_MASK;
      t |= ((uint32_t)sw[i].frames1 << ATON_DST_FNR1_SHIFT)
           & ATON_DST_FNR1_MASK;

      putreg32(t, reg);
    }

  return 0;
}

/****************************************************************************
 * Name: LL_Switch_Deinit
 *
 * Description:
 *   Clear STRSWITCH destination routing entries.
 *
 ****************************************************************************/

int LL_Switch_Deinit(const LL_Switch_DeinitTypeDef *sw, int n)
{
  int i;

  for (i = 0; i < n; i++)
    {
      uintptr_t reg = ATON_STRSWITCH_BASE + ATONN_DSTPORT_ID(sw[i].dest);
      putreg32(0, reg);
    }

  return 0;
}

/****************************************************************************
 * Name: LL_ATON_EnableUnits_Init
 *
 * Description:
 *   Set EN bit in the CTRL register of each specified unit.
 *
 ****************************************************************************/

int LL_ATON_EnableUnits_Init(
    const LL_ATON_EnableUnits_InitTypeDef *units, int n)
{
  int i;
  uint32_t t;
  uintptr_t ctrl_addr;

  for (i = 0; i < n; i++)
    {
      enum AccelUnitsType utype = units[i].unit.unit_type;
      unsigned int uid = units[i].unit.unit_num;

      switch (utype)
        {
          case STRENG:
            ctrl_addr = ATON_STRENG_BASE(uid) + ATON_STRENG_CTRL;
            t = getreg32(ctrl_addr);
            t |= STRENG_CTRL_EN;
            putreg32(t, ctrl_addr);
            break;

          case CONVACC:

            /* Use saved ctrl bits to avoid reading back stale value */

            ctrl_addr = ATON_CONVACC_BASE(uid) + ATON_CONVACC_CTRL;
            putreg32(g_convacc_ctrl[uid] | CONVACC_CTRL_EN, ctrl_addr);
            break;

          case ARITH:
            ctrl_addr = ATON_ARITH_BASE(uid) + ATON_ARITH_CTRL;
            t = getreg32(ctrl_addr);
            t |= ARITH_CTRL_EN;
            putreg32(t, ctrl_addr);
            break;

          case POOL:
            ctrl_addr = ATON_POOL_BASE(uid) + 0x00;
            t = getreg32(ctrl_addr);
            t |= (1 << 0);  /* EN */
            putreg32(t, ctrl_addr);
            break;

          case DECUN:
            ctrl_addr = ATON_DECUN_BASE(uid) + 0x00;
            t = getreg32(ctrl_addr);
            t |= (1 << 0);  /* EN */
            putreg32(t, ctrl_addr);
            break;

          case ACTIV:
            ctrl_addr = ATON_ACTIV_BASE(uid) + 0x00;
            t = getreg32(ctrl_addr);
            t |= (1 << 0);  /* EN */
            putreg32(t, ctrl_addr);
            break;

          case RECBUF:
            ctrl_addr = ATON_RECBUF_BASE(uid) + 0x00;
            t = getreg32(ctrl_addr);
            t |= (1 << 0);  /* EN */
            putreg32(t, ctrl_addr);
            break;

          default:
            break;
        }
    }

  return 0;
}

/****************************************************************************
 * Name: LL_ATON_DisableUnits_Init
 *
 * Description:
 *   Disable each specified unit (CLR+CONFCLR) and turn off its clock gate.
 *
 ****************************************************************************/

int LL_ATON_DisableUnits_Init(
    const LL_ATON_DisableUnits_InitTypeDef *units, int n)
{
  int i;

  for (i = 0; i < n; i++)
    {
      enum AccelUnitsType utype = units[i].unit.unit_type;
      unsigned int uid = units[i].unit.unit_num;

      switch (utype)
        {
          case STRENG:
            aton_disable_clr_confclr(ATON_STRENG_BASE(uid) +
                                     ATON_STRENG_CTRL);
            LL_ATON_DisableClock(ATON_CLKB_STRENG(uid));
            break;

          case CONVACC:
            aton_disable_clr_confclr(ATON_CONVACC_BASE(uid) +
                                     ATON_CONVACC_CTRL);
            LL_ATON_DisableClock(ATON_CLKB_CONVACC(uid));
            break;

          case ARITH:
            aton_disable_clr_confclr(ATON_ARITH_BASE(uid) +
                                     ATON_ARITH_CTRL);
            LL_ATON_DisableClock(ATON_CLKB_ARITH(uid));
            break;

          case POOL:
            aton_disable_clr_confclr(ATON_POOL_BASE(uid) + 0x00);
            LL_ATON_DisableClock(ATON_CLKB_POOL(uid));
            break;

          case DECUN:
            aton_disable_clr_confclr(ATON_DECUN_BASE(uid) + 0x00);
            LL_ATON_DisableClock(ATON_CLKB_DECUN(uid));
            break;

          case ACTIV:
            aton_disable_clr_confclr(ATON_ACTIV_BASE(uid) + 0x00);
            LL_ATON_DisableClock(ATON_CLKB_ACTIV(uid));
            break;

          case RECBUF:
            aton_disable_clr_confclr(ATON_RECBUF_BASE(uid) + 0x00);
            LL_ATON_DisableClock(ATON_CLKB_RECBUF);
            break;

          default:
            return -1;
        }
    }

  return 0;
}

/****************************************************************************
 * Name: LL_Streng_Wait
 *
 * Description:
 *   Poll streaming engines until all engines in the bitmask have completed
 *   (RUNNING bit cleared in CTRL register).
 *
 ****************************************************************************/

int LL_Streng_Wait(uint32_t mask)
{
  int i;
  uint32_t running;

  do
    {
      running = 0;
      for (i = 0; i < ATON_STRENG_NUM; i++)
        {
          if (mask & (1u << i))
            {
              running |= getreg32(ATON_STRENG_BASE(i) + ATON_STRENG_CTRL)
                         & STRENG_CTRL_RUNNING;
            }
        }
    }
  while (running);

  return 0;
}

/****************************************************************************
 * Name: LL_ATON_Cache_MCU_Invalidate_Range
 *
 * Description:
 *   Invalidate MCU D-cache for an address range.
 *   Called from generated model code before output reads.
 *
 ****************************************************************************/

void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t addr, uint32_t size)
{
  up_invalidate_dcache(addr, addr + size);
}
