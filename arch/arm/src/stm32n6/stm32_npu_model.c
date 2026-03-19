/****************************************************************************
 * arch/arm/src/stm32n6/stm32_npu_model.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Wrapper to compile the STEdgeAI-generated model code into the kernel.
 * Named stm32_npu_model.c (not npu_test.c) to avoid a parallel build
 * dependency filename collision with the npu_test app.
 *
 ****************************************************************************/

#include "npu_test.c"
