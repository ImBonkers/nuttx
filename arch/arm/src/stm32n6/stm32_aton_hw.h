/****************************************************************************
 * arch/arm/src/stm32n6/stm32_aton_hw.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Type definitions and function declarations for native ATON NPU
 * register programming. Replicates the exact types from LL_ATON so
 * that STEdgeAI-generated model code compiles unchanged.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32N6_STM32_ATON_HW_H
#define __ARCH_ARM_SRC_STM32N6_STM32_ATON_HW_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Compatibility macros used by generated code */

#define LL_ATON_LIB_UNUSED(x)  ((void)(x))

/* Physical/virtual address translation (identity mapping on STM32N6) */

#ifndef ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR
#  define ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(addr) (addr)
#endif

#ifndef ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR
#  define ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(addr) (addr)
#endif

/* Switch context macros — must match generated designated initializers */

#define LL_Switch_Init_Dest()      .dest
#define LL_Switch_Init_Source(x)   .source##x
#define LL_Switch_Init_Context(x)  .context##x
#define LL_Switch_Init_Frames(x)   .frames##x

/* Source port macros — expand to ATON register source link values.
 * These must match ATON-idxs.h / ATON.h exactly.
 */

#define ATONN_SRCPORT(S, J, U, I, P)   ATON_SRCPORT_##U##_##I##_##P
#define ATONN_SRCPORT_ID(s)             (s)

/* Source port values — STRSWITCH LINK field values */

#define ATON_SRCPORT_STRENG_0_0     0
#define ATON_SRCPORT_STRENG_1_0     1
#define ATON_SRCPORT_STRENG_2_0     2
#define ATON_SRCPORT_STRENG_3_0     3
#define ATON_SRCPORT_STRENG_4_0     4
#define ATON_SRCPORT_STRENG_5_0     5
#define ATON_SRCPORT_STRENG_6_0     6
#define ATON_SRCPORT_STRENG_7_0     7
#define ATON_SRCPORT_STRENG_8_0     8
#define ATON_SRCPORT_STRENG_9_0     9
#define ATON_SRCPORT_CONVACC_0_0    10
#define ATON_SRCPORT_CONVACC_1_0    11
#define ATON_SRCPORT_CONVACC_2_0    12
#define ATON_SRCPORT_CONVACC_3_0    13
#define ATON_SRCPORT_DECUN_0_0      14
#define ATON_SRCPORT_DECUN_1_0      15
#define ATON_SRCPORT_ACTIV_0_0      16
#define ATON_SRCPORT_ACTIV_1_0      17
#define ATON_SRCPORT_ARITH_0_0      18
#define ATON_SRCPORT_ARITH_1_0      19
#define ATON_SRCPORT_ARITH_2_0      20
#define ATON_SRCPORT_ARITH_3_0      21
#define ATON_SRCPORT_POOL_0_0       22
#define ATON_SRCPORT_POOL_1_0       23
#define ATON_SRCPORT_RECBUF_0_0     24
#define ATON_SRCPORT_RECBUF_0_1     25
#define ATON_SRCPORT_RECBUF_0_2     26

/* Destination port macros — expand to byte offsets from STRSWITCH base.
 * The DST register is at STRSWITCH_BASE + offset.
 */

#define ATONN_DSTPORT(S, J, U, I, P)   ATON_DSTPORT_##U##_##I##_##P
#define ATONN_DSTPORT_ID(d)             (d)

/* Destination port offsets: STRSWITCH_BASE + (0x08 + 4*IDX) */

#define ATON_DSTPORT_STRENG_0_0     (0x08 + 4 * 0)
#define ATON_DSTPORT_STRENG_1_0     (0x08 + 4 * 1)
#define ATON_DSTPORT_STRENG_2_0     (0x08 + 4 * 2)
#define ATON_DSTPORT_STRENG_3_0     (0x08 + 4 * 3)
#define ATON_DSTPORT_STRENG_4_0     (0x08 + 4 * 4)
#define ATON_DSTPORT_STRENG_5_0     (0x08 + 4 * 5)
#define ATON_DSTPORT_STRENG_6_0     (0x08 + 4 * 6)
#define ATON_DSTPORT_STRENG_7_0     (0x08 + 4 * 7)
#define ATON_DSTPORT_STRENG_8_0     (0x08 + 4 * 8)
#define ATON_DSTPORT_STRENG_9_0     (0x08 + 4 * 9)
#define ATON_DSTPORT_CONVACC_0_0    (0x08 + 4 * 10)
#define ATON_DSTPORT_CONVACC_0_1    (0x08 + 4 * 11)
#define ATON_DSTPORT_CONVACC_0_2    (0x08 + 4 * 12)
#define ATON_DSTPORT_CONVACC_1_0    (0x08 + 4 * 13)
#define ATON_DSTPORT_CONVACC_1_1    (0x08 + 4 * 14)
#define ATON_DSTPORT_CONVACC_1_2    (0x08 + 4 * 15)
#define ATON_DSTPORT_CONVACC_2_0    (0x08 + 4 * 16)
#define ATON_DSTPORT_CONVACC_2_1    (0x08 + 4 * 17)
#define ATON_DSTPORT_CONVACC_2_2    (0x08 + 4 * 18)
#define ATON_DSTPORT_CONVACC_3_0    (0x08 + 4 * 19)
#define ATON_DSTPORT_CONVACC_3_1    (0x08 + 4 * 20)
#define ATON_DSTPORT_CONVACC_3_2    (0x08 + 4 * 21)
#define ATON_DSTPORT_DECUN_0_0      (0x08 + 4 * 22)
#define ATON_DSTPORT_DECUN_0_1      (0x08 + 4 * 23)
#define ATON_DSTPORT_DECUN_1_0      (0x08 + 4 * 24)
#define ATON_DSTPORT_DECUN_1_1      (0x08 + 4 * 25)
#define ATON_DSTPORT_ACTIV_0_0      (0x08 + 4 * 26)
#define ATON_DSTPORT_ACTIV_1_0      (0x08 + 4 * 27)
#define ATON_DSTPORT_ARITH_0_0      (0x08 + 4 * 28)
#define ATON_DSTPORT_ARITH_0_1      (0x08 + 4 * 29)
#define ATON_DSTPORT_ARITH_1_0      (0x08 + 4 * 30)
#define ATON_DSTPORT_ARITH_1_1      (0x08 + 4 * 31)
#define ATON_DSTPORT_ARITH_2_0      (0x08 + 4 * 32)
#define ATON_DSTPORT_ARITH_2_1      (0x08 + 4 * 33)
#define ATON_DSTPORT_ARITH_3_0      (0x08 + 4 * 34)
#define ATON_DSTPORT_ARITH_3_1      (0x08 + 4 * 35)
#define ATON_DSTPORT_POOL_0_0       (0x08 + 4 * 36)
#define ATON_DSTPORT_POOL_1_0       (0x08 + 4 * 37)
#define ATON_DSTPORT_RECBUF_0_0     (0x08 + 4 * 38)
#define ATON_DSTPORT_RECBUF_0_1     (0x08 + 4 * 39)
#define ATON_DSTPORT_RECBUF_0_2     (0x08 + 4 * 40)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Pointer union — must match ll_aton.h exactly */

typedef union
{
  unsigned char *p;
  uintptr_t i;
} ll_aton_pointer;

/* Unit types */

enum AccelUnitsType
{
  STRENG = 0,
  STRENG64,
  CONVACC,
  DECUN,
  ACTIV,
  ARITH,
  POOL,
  IMC,
  RECBUF,
};

typedef struct
{
  enum AccelUnitsType unit_type;
  unsigned short unit_num;
} AccelUnits;

/* Port types */

typedef unsigned int SourcePort;
typedef unsigned int DestPort;

/* Stream engine tensor init (matches ll_aton.h LL_Streng_TensorInitTypeDef)
 */

typedef struct
{
  unsigned dir : 1;
  unsigned raw : 1;
  unsigned raw_out : 1;
  unsigned continuous : 1;
  unsigned noblk : 1;
  unsigned noinc : 1;
  unsigned align_right : 1;
  unsigned mem_lsb : 1;
  unsigned sync_with_other : 1;
  unsigned nbits_unsigned : 1;
  unsigned bus_cid : 3;
  unsigned cacheable : 1;
  unsigned cache_allocate : 1;
  unsigned bus_pfetch : 1;
  unsigned cache_linesize : 2;
  unsigned cipher_en : 1;
  unsigned key_sel : 1;
  unsigned char sync_dma;
  ll_aton_pointer addr_base;
  unsigned offset_start;
  unsigned offset_end;
  unsigned offset_limit;
  unsigned frame_count;
  unsigned fwidth;
  unsigned fheight;
  unsigned batch_depth;
  unsigned batch_offset;
  unsigned frame_offset;
  unsigned line_offset;
  unsigned loop_offset;
  unsigned frame_loop_cnt;
  unsigned loop_offset2;
  unsigned frame_loop_cnt2;
  unsigned frame_tot_cnt;
  unsigned char nbits_in;
  unsigned char nbits_out;
} LL_Streng_TensorInitTypeDef;

/* Convolution accelerator init
 * (matches ll_aton.h LL_Convacc_InitTypeDef)
 */

typedef enum
{
  AFILT_MODE_NONE       = 0,
  AFILT_MODE_PIXELDROP  = 1,
  AFILT_MODE_FRAMEDROP  = 2,
  AFILT_MODE_FRAMEZERO  = 3
} LL_Convacc_Afilt_Mode;

typedef struct
{
  unsigned rounding_f : 1;
  unsigned saturation_f : 1;
  unsigned round_mode_f : 2;
  unsigned inbytes_f : 2;
  unsigned rounding_o : 1;
  unsigned saturation_o : 1;
  unsigned round_mode_o : 1;
  unsigned relu_mode_o : 1;
  unsigned outbytes_o : 2;
  unsigned simd : 2;
  unsigned accumulate : 1;
  unsigned accumulate_first : 1;
  unsigned accumulate_gen_first : 1;
  unsigned fstat : 1;
  unsigned raw_o : 1;
  unsigned kt1_mode : 1;
  unsigned deepmode : 1;
  unsigned dss2mode : 1;
  unsigned f_unsigned : 1;
  unsigned k_unsigned : 1;
  unsigned kseten : 2;
  unsigned char shift_f;
  unsigned char shift_a;
  unsigned char shift_o;
  unsigned fWidth;
  unsigned fHeight;
  unsigned char kernelWidth;
  unsigned char kernelHeight;
  unsigned char nKernels;
  unsigned short batchDepth;
  unsigned char hstride;
  unsigned char vstride;
  unsigned short left_padding;
  unsigned short right_padding;
  unsigned short top_padding;
  unsigned short bot_padding;
  unsigned short left_crop;
  unsigned short right_crop;
  unsigned short top_crop;
  unsigned short bot_crop;
  unsigned short fstatcnt;
  LL_Convacc_Afilt_Mode afilt_mode;
  unsigned char afilt_tot;
  unsigned char afilt_first;
  unsigned char afilt_last;
  unsigned char kfilt_tot;
  unsigned char kfilt_first;
  unsigned char kfilt_last;
  int fsub;
  unsigned vshift : 2;
  short zfbias;
} LL_Convacc_InitTypeDef;

/* Arithmetic accelerator (matches ll_aton.h LL_Arithacc_InitTypeDef) */

typedef enum
{
  ARITH_AFFINE = 1,
  ARITH_MIN,
  ARITH_MAX,
  ARITH_MUL,
  ARITH_X_AND_Y,
  ARITH_X_OR_Y,
  ARITH_NOT_X,
  ARITH_X_XOR_Y,
  ARITH_X_EQ_Y,
  ARITH_X_LT_Y,
  ARITH_X_LE_Y,
  ARITH_X_GT_Y,
  ARITH_X_GE_Y,
  ARITH_ABS_X,
  ARITH_SIGN_X,
  ARITH_CLIP
} LL_Arithacc_Op;

typedef enum
{
  ARITH_BCAST_NONE,
  ARITH_BCAST_CHAN,
  ARITH_BCAST_HEIGHT,
  ARITH_BCAST_WIDTH,
  ARITH_BCAST_HEIGHT_WIDTH,
  ARITH_BCAST_SCALAR,
} LL_Arithacc_Bcast;

typedef struct
{
  unsigned rounding_x : 1;
  unsigned saturation_x : 1;
  unsigned round_mode_x : 2;
  unsigned inbytes_x : 2;
  unsigned outbytes_x : 2;
  signed char shift_x;
  unsigned rounding_y : 1;
  unsigned saturation_y : 1;
  unsigned round_mode_y : 2;
  unsigned inbytes_y : 2;
  unsigned outbytes_y : 2;
  unsigned combinebc : 1;
  unsigned clipout : 1;
  signed char shift_y;
  unsigned rounding_o : 1;
  unsigned saturation_o : 1;
  unsigned round_mode_o : 1;
  unsigned relu_mode_o : 1;
  unsigned outbytes_o : 2;
  unsigned char shift_o;
  unsigned scalar : 1;
  unsigned dualinput : 1;
  LL_Arithacc_Op operation;
  LL_Arithacc_Bcast bcast;
  unsigned char Ax_shift;
  unsigned char By_shift;
  unsigned char C_shift;
  unsigned fWidth;
  unsigned fHeight;
  unsigned short fChannels;
  unsigned short batchDepth;
  short clipmin;
  short clipmax;
  short A_scalar;
  short B_scalar;
  short C_scalar;
  ll_aton_pointer A_vector;
  ll_aton_pointer B_vector;
  ll_aton_pointer C_vector;
  unsigned char vec_precision[3];
} LL_Arithacc_InitTypeDef;

/* Switch init (matches ll_aton.h LL_Switch_InitTypeDef,
 * ATON_SWITCH_CONTEXT_NUM == 2)
 */

typedef struct
{
  SourcePort source0;
  SourcePort source1;
  DestPort dest;
  unsigned char frames0;
  unsigned char frames1;
  unsigned context0 : 1;
  unsigned context1 : 1;
} LL_Switch_InitTypeDef;

typedef LL_Switch_InitTypeDef LL_Switch_DeinitTypeDef;

/* Activation unit */

typedef enum
{
  ACTIV_RELU = 1,
  ACTIV_PRELU,
  ACTIV_TRELU,
  ACTIV_FUNC,
  ACTIV_LUT
} LL_Activacc_Op;

typedef struct
{
  unsigned rounding_f : 1;
  unsigned saturation_f : 1;
  unsigned round_mode_f : 2;
  unsigned inbytes_f : 2;
  unsigned outbytes_f : 2;
  unsigned rounding_o : 1;
  unsigned saturation_o : 1;
  unsigned round_mode_o : 1;
  unsigned relu_mode_o : 1;
  unsigned outbytes_o : 2;
  unsigned signedop : 1;
  unsigned char shift_f;
  unsigned char shift_o;
  unsigned parameter;
  unsigned parameter_2;
  unsigned nbytes;
  ll_aton_pointer ROM0_vector;
  ll_aton_pointer ROM1_vector;
  ll_aton_pointer LUT_vector;
  unsigned ROM0_nbytes;
  unsigned ROM1_nbytes;
  unsigned char shift_b;
  unsigned char shift_c;
  unsigned char shift_norm;
  unsigned char bwidth;
  int fsub;
  LL_Activacc_Op operation;
} LL_Activacc_InitTypeDef;

int LL_Activacc_Init(int id,
                     const LL_Activacc_InitTypeDef *conf);

/* Pooling unit */

typedef enum
{
  POOL_MAX = 1,
  POOL_MIN,
  POOL_AVG,
  POOL_GMAX,
  POOL_GMIN,
  POOL_GAVG
} LL_Poolacc_Op;

typedef struct
{
  LL_Poolacc_Op operation;
  unsigned avgnopad : 1;
  unsigned short inputX;
  unsigned short inputY;
  unsigned short outputX;
  unsigned short outputY;
  unsigned char poolWinX;
  unsigned char poolWinY;
  unsigned char strideX;
  unsigned char strideY;
  unsigned short topCrop;
  unsigned short bottomCrop;
  unsigned short leftCrop;
  unsigned short rightCrop;
  unsigned short topPad;
  unsigned short bottomPad;
  unsigned short leftPad;
  unsigned short rightPad;
  unsigned short batchSize;
  unsigned char shift_f;
  unsigned char shift_o;
  unsigned dualLine : 1;
  unsigned nbytes : 2;
  unsigned rounding_f : 1;
  unsigned saturation_f : 1;
  unsigned round_mode_f : 2;
  unsigned inbytes_f : 2;
  unsigned outbytes_f : 2;
  unsigned rounding_o : 1;
  unsigned saturation_o : 1;
  unsigned round_mode_o : 1;
  unsigned relu_mode_o : 1;
  unsigned outbytes_o : 2;
  short mulval;
  unsigned pad_val_en : 1;
  short pad_val;
} LL_Poolacc_InitTypeDef;

int LL_Poolacc_Init(int id, const LL_Poolacc_InitTypeDef *conf);

/* Enable/Disable units */

typedef struct
{
  AccelUnits unit;
} LL_ATON_EnableUnits_InitTypeDef;

typedef LL_ATON_EnableUnits_InitTypeDef LL_ATON_DisableUnits_InitTypeDef;

/* Epoch block function pointer */

typedef void (*EpochBlock_FuncPtr_t)(const void *epoch_block);

/* Epoch block flags */

typedef enum
{
  EpochBlock_Flags_NONE           = 0x0,
  EpochBlock_Flags_epoch_start    = (0x1 << 0),
  EpochBlock_Flags_epoch_end      = (0x1 << 1),
  EpochBlock_Flags_blob           = (0x1 << 2),
  EpochBlock_Flags_last_eb        = (0x1 << 3),
  EpochBlock_Flags_pure_hw        = (0x1 << 4),
  EpochBlock_Flags_pure_sw        = (0x1 << 5),
  EpochBlock_Flags_hybrid         = (0x1 << 6),
  EpochBlock_Flags_internal       = (0x1 << 7),
  EpochBlock_Flags_blob_encrypted = (0x1 << 8)
} EpochBlock_Flags_t;

/* Epoch block item (matches ll_aton_NN_interface.h) */

typedef struct
{
  EpochBlock_FuncPtr_t start_epoch_block;
  EpochBlock_FuncPtr_t end_epoch_block;
  uintptr_t blob_address;
  uint32_t wait_mask;
  uint16_t flags;
} EpochBlock_ItemTypeDef;

/* Buffer info types (for input/output buffer metadata) */

typedef union
{
  unsigned char *p;
  uintptr_t i;
} __LL_address_t;

typedef enum
{
  DataType_UNDEFINED = 0,
  DataType_FLOAT     = 1,
  DataType_UINT8     = 6,
  DataType_INT8      = 7,
  DataType_UINT16    = 8,
  DataType_INT16     = 9,
  DataType_INT32     = 10,
  DataType_FXP       = 11,
  DataType_FLOAT16   = 14,
} Buffer_DataType_TypeDef;

typedef enum
{
  CHPos_UNDEFINED = 0,
  CHPos_First     = 1,
  CHPos_Last      = 2,
  CHPos_Mixed     = 3,
} Buffer_CHPos_TypeDef;

typedef struct
{
  const char *name;
  __LL_address_t addr_base;
  uint32_t offset_start;
  uint32_t offset_end;
  uint32_t offset_limit;
  uint8_t is_user_allocated;
  uint8_t is_param;
  uint16_t epoch;
  uint32_t batch;
  const uint32_t *mem_shape;
  uint16_t mem_ndims;
  Buffer_CHPos_TypeDef chpos;
  Buffer_DataType_TypeDef type;
  int8_t Qm;
  int8_t Qn;
  uint8_t Qunsigned;
  uint8_t ndims;
  uint8_t nbits;
  uint8_t per_channel;
  const uint32_t *shape;
  const float *scale;
  const int16_t *offset;
} LL_Buffer_InfoTypeDef;

/* Encryption typedef (referenced by generated code) */

typedef struct
{
  unsigned int enable;
  uint64_t encryption_id;
  unsigned int rounds;
  unsigned int key_sel;
  unsigned int increment;
} LL_Streng_EncryptionTypedef;

/* User IO result (referenced by generated code) */

typedef enum
{
  LL_ATON_User_IO_NOERROR,
  LL_ATON_User_IO_WRONG_ALIGN,
  LL_ATON_User_IO_WRONG_SIZE,
  LL_ATON_User_IO_WRONG_INDEX,
} LL_ATON_User_IO_Result_t;

/****************************************************************************
 * Inline helpers for LL_Buffer_InfoTypeDef
 ****************************************************************************/

static inline unsigned char *LL_Buffer_addr_base(
    const LL_Buffer_InfoTypeDef *buf)
{
  if (buf->is_user_allocated)
    {
      unsigned char **tmp = (unsigned char **)buf->addr_base.p;
      return *tmp;
    }

  return buf->addr_base.p;
}

static inline unsigned char *LL_Buffer_addr_start(
    const LL_Buffer_InfoTypeDef *buf)
{
  return LL_Buffer_addr_base(buf) + buf->offset_start;
}

static inline unsigned char *LL_Buffer_addr_end(
    const LL_Buffer_InfoTypeDef *buf)
{
  return LL_Buffer_addr_base(buf) + buf->offset_end;
}

static inline uint32_t LL_Buffer_len(const LL_Buffer_InfoTypeDef *buf)
{
  return buf->offset_end - buf->offset_start;
}

/****************************************************************************
 * Inline helpers for LL_Streng_TensorInitTypeDef
 ****************************************************************************/

static inline unsigned char *LL_Streng_addr_start(
    const LL_Streng_TensorInitTypeDef *conf)
{
  return conf->addr_base.p + conf->offset_start;
}

static inline unsigned char *LL_Streng_addr_end(
    const LL_Streng_TensorInitTypeDef *conf)
{
  return conf->addr_base.p + conf->offset_end;
}

static inline uint32_t LL_Streng_len(
    const LL_Streng_TensorInitTypeDef *conf)
{
  return conf->offset_end - conf->offset_start;
}

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int LL_Streng_TensorInit(int id,
                         const LL_Streng_TensorInitTypeDef *conf, int n);
int LL_Convacc_Init(int id, const LL_Convacc_InitTypeDef *conf);
int LL_Arithacc_Init(int id, const LL_Arithacc_InitTypeDef *conf);
int LL_Switch_Init(const LL_Switch_InitTypeDef *sw, int n);
int LL_Switch_Deinit(const LL_Switch_DeinitTypeDef *sw, int n);
int LL_ATON_EnableUnits_Init(
    const LL_ATON_EnableUnits_InitTypeDef *units, int n);
int LL_ATON_DisableUnits_Init(
    const LL_ATON_DisableUnits_InitTypeDef *units, int n);
int LL_Streng_Wait(uint32_t mask);
void LL_ATON_EnableClock(unsigned int clock);
void LL_ATON_DisableClock(unsigned int clock);
void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t addr, uint32_t size);
void LL_ATON_Cache_MCU_Clean_Range(uintptr_t addr, uint32_t size);

#endif /* __ARCH_ARM_SRC_STM32N6_STM32_ATON_HW_H */
