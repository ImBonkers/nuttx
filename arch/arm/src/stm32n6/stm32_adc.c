/****************************************************************************
 * arch/arm/src/stm32n6/stm32_adc.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32_adc.h"
#include "hardware/stm32n6xxx_adc.h"
#include "hardware/stm32n6xxx_rcc.h"

#if defined(CONFIG_STM32N6_ADC1_DMA) || defined(CONFIG_STM32N6_ADC2_DMA)
#  include <nuttx/cache.h>
#  include "stm32_dma.h"
#  include "stm32_dcache.h"
#endif

#ifdef CONFIG_STM32N6_ADC

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define STM32_IRQ_ADC12        (16 + 46)

/* Default sample time: 246.5 ADC clock cycles at 64MHz = ~3.9us.
 * STM32N6 sample times: 1.5/2.5/6.5/11.5/23.5/46.5/246.5/1499.5
 */

#define ADC_DEFAULT_SMPR       6  /* 246.5 cycles */

#define ADC_TIMEOUT            100000

/* PWR SVMCR3 ASV bit — mandatory for ADC analog section */

#define PWR_SVMCR3_ASV         (1 << 12)

/* ADC DMA convenience macro */

#if defined(CONFIG_STM32N6_ADC1_DMA) || defined(CONFIG_STM32N6_ADC2_DMA)
#  define ADC_HAVE_DMA
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_adc_dev_s
{
  const struct adc_callback_s *cb;
  uint32_t base;
  uint8_t  intf;
  uint8_t  nchannels;
  uint8_t  current;
  uint8_t  chanlist[STM32_ADC_MAX_SAMPLES];

#ifdef ADC_HAVE_DMA
  bool     hasdma;
  uint8_t  dmareq;
  DMA_HANDLE dma;
  uint32_t dmabuf[STM32_ADC_MAX_SAMPLES] aligned_data(32);
#endif
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static uint32_t adc_getreg(struct stm32_adc_dev_s *priv, int offset);
static void adc_putreg(struct stm32_adc_dev_s *priv, int offset,
                       uint32_t value);

static void adc_enable(struct stm32_adc_dev_s *priv);
static void adc_disable(struct stm32_adc_dev_s *priv);
static void adc_configure(struct stm32_adc_dev_s *priv);

static int  adc_bind(struct adc_dev_s *dev,
                     const struct adc_callback_s *callback);
static void adc_reset(struct adc_dev_s *dev);
static int  adc_setup(struct adc_dev_s *dev);
static void adc_shutdown(struct adc_dev_s *dev);
static void adc_rxint(struct adc_dev_s *dev, bool enable);
static int  adc_ioctl(struct adc_dev_s *dev, int cmd, unsigned long arg);
static int  adc_interrupt(int irq, void *context, void *arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct adc_ops_s g_adcops =
{
  .ao_bind     = adc_bind,
  .ao_reset    = adc_reset,
  .ao_setup    = adc_setup,
  .ao_shutdown = adc_shutdown,
  .ao_rxint    = adc_rxint,
  .ao_ioctl    = adc_ioctl,
};

#ifdef CONFIG_STM32N6_ADC1
static struct stm32_adc_dev_s g_adcpriv1 =
{
  .base  = STM32_ADC1_BASE,
  .intf  = 1,
#ifdef CONFIG_STM32N6_ADC1_DMA
  .hasdma = true,
  .dmareq = GPDMA_REQ_ADC1,
#endif
};

static struct adc_dev_s g_adcdev1 =
{
  .ad_ops  = &g_adcops,
  .ad_priv = &g_adcpriv1,
};
#endif

#ifdef CONFIG_STM32N6_ADC2
static struct stm32_adc_dev_s g_adcpriv2 =
{
  .base  = STM32_ADC2_BASE,
  .intf  = 2,
#ifdef CONFIG_STM32N6_ADC2_DMA
  .hasdma = true,
  .dmareq = GPDMA_REQ_ADC2,
#endif
};

static struct adc_dev_s g_adcdev2 =
{
  .ad_ops  = &g_adcops,
  .ad_priv = &g_adcpriv2,
};
#endif

static bool g_adc_irq_attached;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t adc_getreg(struct stm32_adc_dev_s *priv, int offset)
{
  return getreg32(priv->base + offset);
}

static void adc_putreg(struct stm32_adc_dev_s *priv, int offset,
                       uint32_t value)
{
  putreg32(value, priv->base + offset);
}

/****************************************************************************
 * Name: adc_enable
 *
 * Description:
 *   Enable the ADC.  Write ONLY ADEN to CR — never read-modify-write CR
 *   because it has write-1-to-trigger bits (ADCAL, ADSTART, etc.).
 *
 ****************************************************************************/

static void adc_enable(struct stm32_adc_dev_s *priv)
{
  int timeout;

  adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_ADRDY);
  adc_putreg(priv, STM32_ADC_CR_OFFSET, ADC_CR_ADEN);

  for (timeout = 0; timeout < ADC_TIMEOUT; timeout++)
    {
      if ((adc_getreg(priv, STM32_ADC_ISR_OFFSET) & ADC_ISR_ADRDY) != 0)
        {
          return;
        }
    }

  aerr("ADC%d: ADRDY timeout\n", priv->intf);
}

/****************************************************************************
 * Name: adc_disable
 ****************************************************************************/

static void adc_disable(struct stm32_adc_dev_s *priv)
{
  uint32_t cr;
  int timeout;

  cr = adc_getreg(priv, STM32_ADC_CR_OFFSET);
  if ((cr & ADC_CR_ADEN) == 0)
    {
      return;
    }

  if ((cr & ADC_CR_ADSTART) != 0)
    {
      adc_putreg(priv, STM32_ADC_CR_OFFSET, ADC_CR_ADEN | ADC_CR_ADSTP);
      for (timeout = 0; timeout < ADC_TIMEOUT; timeout++)
        {
          if ((adc_getreg(priv, STM32_ADC_CR_OFFSET) & ADC_CR_ADSTP) == 0)
            {
              break;
            }
        }
    }

  adc_putreg(priv, STM32_ADC_CR_OFFSET, ADC_CR_ADEN | ADC_CR_ADDIS);
  for (timeout = 0; timeout < ADC_TIMEOUT; timeout++)
    {
      if ((adc_getreg(priv, STM32_ADC_CR_OFFSET) & ADC_CR_ADEN) == 0)
        {
          break;
        }
    }
}

/****************************************************************************
 * Name: adc_configure
 ****************************************************************************/

static void adc_configure(struct stm32_adc_dev_s *priv)
{
  uint32_t pcsel = 0;
  uint32_t smpr1 = 0;
  uint32_t smpr2 = 0;
  uint32_t sqr1 = 0;
  int i;

  for (i = 0; i < priv->nchannels; i++)
    {
      uint8_t ch = priv->chanlist[i];

      pcsel |= ADC_PCSEL_PCSEL(ch);

      if (ch < 10)
        {
          smpr1 |= (uint32_t)ADC_DEFAULT_SMPR << ADC_SMPR1_SMP_SHIFT(ch);
        }
      else if (ch < 20)
        {
          smpr2 |= (uint32_t)ADC_DEFAULT_SMPR << ADC_SMPR2_SMP_SHIFT(ch);
        }
    }

  sqr1 = ((priv->nchannels - 1) & 0xf) << ADC_SQR1_L_SHIFT;

  for (i = 0; i < priv->nchannels && i < 4; i++)
    {
      int shift = 6 * (i + 1);
      sqr1 |= (uint32_t)(priv->chanlist[i] & 0x1f) << shift;
    }

  adc_putreg(priv, STM32_ADC_PCSEL_OFFSET, pcsel);
  adc_putreg(priv, STM32_ADC_SMPR1_OFFSET, smpr1);
  adc_putreg(priv, STM32_ADC_SMPR2_OFFSET, smpr2);
  adc_putreg(priv, STM32_ADC_SQR1_OFFSET, sqr1);
  adc_putreg(priv, STM32_ADC_SQR2_OFFSET, 0);
  adc_putreg(priv, STM32_ADC_SQR3_OFFSET, 0);
  adc_putreg(priv, STM32_ADC_SQR4_OFFSET, 0);

  /* CFGR1: 12-bit, software trigger, overwrite on overrun.
   * When DMA is enabled, set DMNGT to DMA one-shot mode.
   */

  {
    uint32_t cfgr1 = ADC_CFGR1_RES_12BIT | ADC_CFGR1_OVRMOD;
#ifdef ADC_HAVE_DMA
    if (priv->hasdma && priv->dma != NULL)
      {
        cfgr1 |= ADC_CFGR1_DMNGT_DMA1;
      }
#endif
    adc_putreg(priv, STM32_ADC_CFGR1_OFFSET, cfgr1);
  }
  adc_putreg(priv, STM32_ADC_CFGR2_OFFSET, 0);

  /* CCR: independent mode, enable VREFINT */

  putreg32(ADC_CCR_DUAL_INDEP | ADC_CCR_VREFEN, STM32_ADC12_CCR);
}

/****************************************************************************
 * Name: adc_bind
 ****************************************************************************/

static int adc_bind(struct adc_dev_s *dev,
                    const struct adc_callback_s *callback)
{
  struct stm32_adc_dev_s *priv = (struct stm32_adc_dev_s *)dev->ad_priv;
  priv->cb = callback;
  return OK;
}

/****************************************************************************
 * Name: adc_reset
 ****************************************************************************/

static void adc_reset(struct adc_dev_s *dev)
{
  adc_setup(dev);
}

/****************************************************************************
 * Name: adc_setup
 *
 * Description:
 *   Full hardware initialization on first open.
 *
 ****************************************************************************/

static int adc_setup(struct adc_dev_s *dev)
{
  struct stm32_adc_dev_s *priv = (struct stm32_adc_dev_s *)dev->ad_priv;

  /* Enable VDDA supply valid — MANDATORY for ADC analog converter.
   * Without this, ADC digital logic works (ADRDY, EOC) but DR is
   * always zero.  PWR SVMCR3 offset = 0x3C, bit 12 = ASV.
   */

  modifyreg32(STM32_PWR_BASE + 0x3c, 0, PWR_SVMCR3_ASV);

  /* Enable ADC12 bus clock */

  putreg32(RCC_AHB1ENR_ADC12EN, STM32_RCC_AHB1ENSR);

  /* Set ADC12 kernel clock to HSI (64 MHz) */

  modifyreg32(STM32_RCC_CCIPR1,
              RCC_CCIPR1_ADC12SEL_MASK,
              RCC_CCIPR1_ADC12SEL_HSI);

  /* Exit deep power down */

  adc_putreg(priv, STM32_ADC_CR_OFFSET, 0);
  up_udelay(20);

  /* Allocate DMA channel if configured */

#ifdef ADC_HAVE_DMA
  if (priv->hasdma && priv->dma == NULL)
    {
      priv->dma = stm32_dmachannel(GPDMA_TTYPE_P2M);
      if (priv->dma == NULL)
        {
          priv->hasdma = false;
          aerr("ADC%d: DMA channel unavailable, using polling\n",
               priv->intf);
        }
    }
#endif

  /* Configure channels (must be done with ADEN=0) */

  adc_configure(priv);

  /* Attach ISR */

  if (!g_adc_irq_attached)
    {
      irq_attach(STM32_IRQ_ADC12, adc_interrupt, NULL);
      g_adc_irq_attached = true;
    }

  /* Enable ADC */

  adc_enable(priv);

  /* Clear pending flags, enable NVIC */

  adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_ALLINTS);
  up_enable_irq(STM32_IRQ_ADC12);

  return OK;
}

/****************************************************************************
 * Name: adc_shutdown
 ****************************************************************************/

static void adc_shutdown(struct adc_dev_s *dev)
{
  struct stm32_adc_dev_s *priv = (struct stm32_adc_dev_s *)dev->ad_priv;

  adc_putreg(priv, STM32_ADC_IER_OFFSET, 0);
  adc_disable(priv);
  adc_putreg(priv, STM32_ADC_CR_OFFSET, ADC_CR_DEEPPWD);

#ifdef ADC_HAVE_DMA
  if (priv->dma != NULL)
    {
      stm32_dmafree(priv->dma);
      priv->dma = NULL;
    }
#endif
}

/****************************************************************************
 * Name: adc_rxint
 ****************************************************************************/

static void adc_rxint(struct adc_dev_s *dev, bool enable)
{
  struct stm32_adc_dev_s *priv = (struct stm32_adc_dev_s *)dev->ad_priv;

  if (enable)
    {
      adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_ALLINTS);
      adc_putreg(priv, STM32_ADC_IER_OFFSET,
                 ADC_IER_EOCIE | ADC_IER_EOSIE | ADC_IER_OVRIE);
    }
  else
    {
      adc_putreg(priv, STM32_ADC_IER_OFFSET, 0);
    }
}

/****************************************************************************
 * Name: adc_ioctl
 *
 * Description:
 *   ANIOC_TRIGGER starts a software-triggered conversion sequence.
 *   Uses polled mode to avoid ISR/FIFO race conditions.
 *
 ****************************************************************************/

static int adc_ioctl(struct adc_dev_s *dev, int cmd, unsigned long arg)
{
  struct stm32_adc_dev_s *priv = (struct stm32_adc_dev_s *)dev->ad_priv;

  switch (cmd)
    {
      case ANIOC_TRIGGER:
        {
          int i;

          /* Disable IER to prevent ISR from stealing DR reads */

          adc_putreg(priv, STM32_ADC_IER_OFFSET, 0);
          adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_ALLINTS);

#ifdef ADC_HAVE_DMA
          if (priv->hasdma && priv->dma != NULL && priv->nchannels > 0)
            {
              struct stm32_gpdma_cfg_s dmacfg;
              size_t nbytes = priv->nchannels * sizeof(uint32_t);
              size_t nbytes_aligned = (nbytes + 31) & ~31u;
              int ret;

              /* Clean+invalidate D-cache before DMA (set/way — MVA ops
               * fail on M55 Secure).
               */

              stm32_dcache_clean_invalidate();

              /* Build DMA config: ADC DR → dmabuf (P2M, word width) */

              memset(&dmacfg, 0, sizeof(dmacfg));
              dmacfg.src_addr   = priv->base + STM32_ADC_DR_OFFSET;
              dmacfg.dest_addr  = (uint32_t)(uintptr_t)priv->dmabuf;
              dmacfg.tr1        = GPDMA_CXTR1_SDW_LOG2_WORD |
                                  GPDMA_CXTR1_DDW_LOG2_WORD |
                                  GPDMA_CXTR1_DINC;
              dmacfg.request    = GPDMA_CXTR2_REQSEL(priv->dmareq);
              dmacfg.ntransfers = nbytes;

              /* Clear any pending DMA requests by toggling DMNGT off
               * then on.  The ADC may have a stale EOC from a prior
               * polling conversion that would cause an immediate
               * spurious DMA transfer.  ADSTP first if running.
               */

              {
                uint32_t cr = adc_getreg(priv, STM32_ADC_CR_OFFSET);
                if (cr & ADC_CR_ADSTART)
                  {
                    adc_putreg(priv, STM32_ADC_CR_OFFSET,
                               ADC_CR_ADEN | ADC_CR_ADSTP);
                    while (adc_getreg(priv, STM32_ADC_CR_OFFSET) &
                           ADC_CR_ADSTP);
                  }
              }

              adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_ALLINTS);

              stm32_dmasetup(priv->dma, &dmacfg);
              stm32_dmastart(priv->dma, NULL, NULL, false);

              /* Start conversion sequence */

              priv->current = 0;
              adc_putreg(priv, STM32_ADC_CR_OFFSET,
                         ADC_CR_ADEN | ADC_CR_ADSTART);

              /* Poll for DMA completion (generous timeout) */

              ret = stm32_dmapollwait(priv->dma, 50000);
              stm32_dmastop(priv->dma);

              if (ret < 0)
                {
                  aerr("ADC%d: DMA %s\n", priv->intf,
                       ret == -ETIMEDOUT ? "timeout" : "error");
                }
              else
                {
                  /* Clean+invalidate D-cache after DMA */

                  stm32_dcache_clean_invalidate();

                  for (i = 0; i < priv->nchannels; i++)
                    {
                      if (priv->cb != NULL)
                        {
                          priv->cb->au_receive(dev,
                                               priv->chanlist[i],
                                               (int32_t)priv->dmabuf[i]);
                        }
                    }
                }
            }
          else
#endif
            {
              int timeout;

              /* Start conversion */

              priv->current = 0;
              adc_putreg(priv, STM32_ADC_CR_OFFSET,
                         ADC_CR_ADEN | ADC_CR_ADSTART);

              /* Poll each channel */

              for (i = 0; i < priv->nchannels; i++)
                {
                  for (timeout = 0; timeout < ADC_TIMEOUT; timeout++)
                    {
                      if (adc_getreg(priv, STM32_ADC_ISR_OFFSET) &
                          ADC_ISR_EOC)
                        {
                          break;
                        }
                    }

                  uint32_t dr = adc_getreg(priv, STM32_ADC_DR_OFFSET);

                  if (priv->cb != NULL)
                    {
                      priv->cb->au_receive(dev,
                                           priv->chanlist[i],
                                           (int32_t)dr);
                    }
                }
            }

          /* Clear EOS, re-enable IER */

          adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_ALLINTS);
          adc_putreg(priv, STM32_ADC_IER_OFFSET,
                     ADC_IER_EOCIE | ADC_IER_EOSIE | ADC_IER_OVRIE);
          return OK;
        }

      default:
        return -ENOTTY;
    }
}

/****************************************************************************
 * Name: adc_interrupt
 ****************************************************************************/

static int adc_interrupt(int irq, void *context, void *arg)
{
  UNUSED(irq);
  UNUSED(context);
  UNUSED(arg);

#ifdef CONFIG_STM32N6_ADC1
  {
    struct stm32_adc_dev_s *priv = &g_adcpriv1;
    uint32_t isr = adc_getreg(priv, STM32_ADC_ISR_OFFSET);

    if ((isr & ADC_ISR_OVR) != 0)
      {
        adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_OVR);
      }

    if ((isr & ADC_ISR_EOC) != 0)
      {
        uint32_t data = adc_getreg(priv, STM32_ADC_DR_OFFSET);
        int ch_idx = priv->current;

        if (ch_idx < priv->nchannels && priv->cb != NULL)
          {
            priv->cb->au_receive(&g_adcdev1,
                                 priv->chanlist[ch_idx],
                                 (int32_t)data);
          }

        priv->current++;
      }

    if ((isr & ADC_ISR_EOS) != 0)
      {
        adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_EOS);
      }
  }
#endif

#ifdef CONFIG_STM32N6_ADC2
  {
    struct stm32_adc_dev_s *priv = &g_adcpriv2;
    uint32_t isr = adc_getreg(priv, STM32_ADC_ISR_OFFSET);

    if ((isr & ADC_ISR_OVR) != 0)
      {
        adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_OVR);
      }

    if ((isr & ADC_ISR_EOC) != 0)
      {
        uint32_t data = adc_getreg(priv, STM32_ADC_DR_OFFSET);
        int ch_idx = priv->current;

        if (ch_idx < priv->nchannels && priv->cb != NULL)
          {
            priv->cb->au_receive(&g_adcdev2,
                                 priv->chanlist[ch_idx],
                                 (int32_t)data);
          }

        priv->current++;
      }

    if ((isr & ADC_ISR_EOS) != 0)
      {
        adc_putreg(priv, STM32_ADC_ISR_OFFSET, ADC_ISR_EOS);
      }
  }
#endif

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

struct adc_dev_s *stm32n6_adc_initialize(int intf,
                                         const uint8_t *chanlist,
                                         int nchannels)
{
  struct adc_dev_s *dev;
  struct stm32_adc_dev_s *priv;
  int i;

  switch (intf)
    {
#ifdef CONFIG_STM32N6_ADC1
      case 1:
        dev  = &g_adcdev1;
        priv = &g_adcpriv1;
        break;
#endif

#ifdef CONFIG_STM32N6_ADC2
      case 2:
        dev  = &g_adcdev2;
        priv = &g_adcpriv2;
        break;
#endif

      default:
        return NULL;
    }

  if (nchannels > STM32_ADC_MAX_SAMPLES)
    {
      nchannels = STM32_ADC_MAX_SAMPLES;
    }

  priv->nchannels = nchannels;
  for (i = 0; i < nchannels; i++)
    {
      priv->chanlist[i] = chanlist[i];
    }

  return dev;
}

#endif /* CONFIG_STM32N6_ADC */
