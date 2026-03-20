/****************************************************************************
 * arch/arm/src/stm32n6/stm32_aton_hw.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Platform glue for the ST LL_ATON runtime on NuttX.
 * The actual NPU register programming is done by the ST SDK's ll_aton.c.
 * This file only provides the NuttX-specific platform stubs.
 *
 ****************************************************************************/

/* This file is intentionally empty — all platform functions are provided
 * by the compat layer (npu/compat/) and the ST LL_ATON runtime (ll_aton.c).
 *
 * Previous implementation had hand-written NPU register programming
 * (LL_Streng_TensorInit, LL_Convacc_Init, etc.) which was replaced by
 * the ST SDK's battle-tested ll_aton.c to support complex models with
 * pipelined CONVACC accumulate.
 */
