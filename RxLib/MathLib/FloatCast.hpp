#ifndef FloatCast_h__
#define FloatCast_h__

#ifdef USE_SSE
    #include <pmmintrin.h>  // SSE3 including
#endif // USE_SSE

#include <assert.h>

/*
SAMPLE RUN
  
~/mod -> g++-3.2 fpu.cpp
~/mod -> ./a.out

ANSI slow, default FPU, float 1.80000 int 1
fast, default FPU, float 1.80000 int 2
ANSI slow, modified FPU, float 1.80000 int 1
fast, modified FPU, float 1.80000 int 1

ANSI slow, default FPU, float 1.80000 int 1
fast, default FPU, float 1.80000 int 2
ANSI slow, modified FPU, float 1.80000 int 1
fast, modified FPU, float 1.80000 int 1

ANSI slow, default FPU, float 1.10000 int 1
fast, default FPU, float 1.10000 int 1
ANSI slow, modified FPU, float 1.10000 int 1
fast, modified FPU, float 1.10000 int 1

ANSI slow, default FPU, float -1.80000 int -1
fast, default FPU, float -1.80000 int -2
ANSI slow, modified FPU, float -1.80000 int -1
fast, modified FPU, float -1.80000 int -1

ANSI slow, default FPU, float -1.10000 int -1
fast, default FPU, float -1.10000 int -1
ANSI slow, modified FPU, float -1.10000 int -1
fast, modified FPU, float -1.10000 int -1
*/


/** 
 * bits to set the floating point control word register
 *
 * Sections 4.9, 8.1.4, 10.2.2 and 11.5 in 
 * IA-32 Intel Architecture Software Developer's Manual
 *   Volume 1: Basic Architecture
 *
 * http://www.intel.com/design/pentium4/manuals/245471.htm
 *
 * http://www.geisswerks.com/ryan/FAQS/fpu.html
 *
 * windows has _controlfp() but it takes different parameters
 *
 * 0 : IM invalid operation mask
 * 1 : DM denormalized operand mask
 * 2 : ZM divide by zero mask
 * 3 : OM overflow mask
 * 4 : UM underflow mask
 * 5 : PM precision, inexact mask
 * 6,7 : reserved
 * 8,9 : PC precision control
 * 10,11 : RC rounding control
 *
 * precision control:
 * 00 : single precision
 * 01 : reserved
 * 10 : double precision
 * 11 : extended precision
 *
 * rounding control:
 * 00 = Round to nearest whole number. (default)
 * 01 = Round down, toward -infinity.
 * 10 = Round up, toward +infinity.
 * 11 = Round toward zero (truncate).
 */
#define __FPU_CW_EXCEPTION_MASK__   (0x003f)
#define __FPU_CW_INVALID__          (0x0001)
#define __FPU_CW_DENORMAL__         (0x0002)
#define __FPU_CW_ZERODIVIDE__       (0x0004)
#define __FPU_CW_OVERFLOW__         (0x0008)
#define __FPU_CW_UNDERFLOW__        (0x0010)
#define __FPU_CW_INEXACT__          (0x0020)

#define __FPU_CW_PREC_MASK__        (0x0300)
#define __FPU_CW_PREC_SINGLE__      (0x0000)
#define __FPU_CW_PREC_DOUBLE__      (0x0200)
#define __FPU_CW_PREC_EXTENDED__    (0x0300)

#define __FPU_CW_ROUND_MASK__       (0x0c00)
#define __FPU_CW_ROUND_NEAR__       (0x0000)
#define __FPU_CW_ROUND_DOWN__       (0x0400)
#define __FPU_CW_ROUND_UP__         (0x0800)
#define __FPU_CW_ROUND_CHOP__       (0x0c00)

#define __FPU_CW_MASK_ALL__         (0x1f3f)


#define __SSE_CW_FLUSHZERO__        (0x8000)
    
#define __SSE_CW_ROUND_MASK__       (0x6000)
#define __SSE_CW_ROUND_NEAR__       (0x0000)
#define __SSE_CW_ROUND_DOWN__       (0x2000)
#define __SSE_CW_ROUND_UP__         (0x4000)
#define __SSE_CW_ROUND_CHOP__       (0x6000)

#define __SSE_CW_EXCEPTION_MASK__   (0x1f80)
#define __SSE_CW_PRECISION__        (0x1000)
#define __SSE_CW_UNDERFLOW__        (0x0800)
#define __SSE_CW_OVERFLOW__         (0x0400)
#define __SSE_CW_DIVIDEZERO__       (0x0200)
#define __SSE_CW_DENORMAL__         (0x0100)
#define __SSE_CW_INVALID__          (0x0080)
// not on all SSE machines
// #define __SSE_CW_DENORMALZERO__     (0x0040)

#define __SSE_CW_MASK_ALL__         (0xffc0)

#define __MOD_FPU_CW_DEFAULT__ \
    (__FPU_CW_EXCEPTION_MASK__ + __FPU_CW_PREC_DOUBLE__ + __FPU_CW_ROUND_CHOP__)

#define __MOD_SSE_CW_DEFAULT__ \
    (__SSE_CW_EXCEPTION_MASK__ + __SSE_CW_ROUND_CHOP__ + __SSE_CW_FLUSHZERO__)



#ifdef USE_SSE
inline unsigned int getSSEStateX86(void);
inline void setSSEModDefault(unsigned int control);
inline void modifySSEStateX86(unsigned int control, unsigned int mask);
#endif // USE_SSE


inline void setRoundingMode(unsigned int round);

inline unsigned int getFPUStateX86(void);
inline void setFPUStateX86(unsigned int control);

inline void setFPUModDefault(void);
inline void assertFPUModDefault(void);

inline void modifyFPUStateX86(const unsigned int control, const unsigned int mask);

inline int FastFtol(const float a);

// assume for now that we are running on an x86
// #ifdef __i386__

#ifdef USE_SSE

inline
unsigned int
getSSEStateX86
    (void)
{
    return _mm_getcsr();
}

inline
void
setSSEStateX86
    (unsigned int control)
{
    _mm_setcsr(control);
}


inline
void
modifySSEStateX86
    (unsigned int control, unsigned int mask)
{
    unsigned int oldControl = getFPUStateX86();
    unsigned int newControl = ((oldControl & (~mask)) | (control & mask));
    setFPUStateX86(newControl);
}


inline
void
setSSEModDefault
    (void)
{
    modifySSEStateX86(__MOD_SSE_CW_DEFAULT__, __SSE_CW_MASK_ALL__);
}    
#endif // USE_SSE


inline
void
setRoundingMode
    (unsigned int round)
{
    assert(round < 4);
    unsigned int mask = 0x3;

    unsigned int fpuControl = getFPUStateX86();
    fpuControl &= ~(mask << 10);
    fpuControl |= round << 10;
    setFPUStateX86(fpuControl);

#ifdef USE_SSE
    unsigned int sseControl = getSSEStateX86();
    sseControl &= ~(mask << 13);
    sseControl |= round << 13;
    setSSEStateX86(sseControl);
#endif // USE_SSE
}


inline
unsigned int
getFPUStateX86
    (void)
{
    unsigned int control = 0;
#if defined(_MSVC)
    __asm fnstcw control;
#elif defined(__GNUG__)
    __asm__ __volatile__ ("fnstcw %0" : "=m" (control));
#endif
    return control;
}



/* set status */
inline
void
setFPUStateX86
    (unsigned int control)
{
#if defined(_MSVC)
    __asm fldcw control;
#elif defined(__GNUG__)
    __asm__ __volatile__ ("fldcw %0" : : "m" (control));
#endif
}


inline
void
modifyFPUStateX86
    (const unsigned int control, const unsigned int mask)
{
    unsigned int oldControl = getFPUStateX86();
    unsigned int newControl = ((oldControl & (~mask)) | (control & mask));
    setFPUStateX86(newControl);
}


inline
void
setFPUModDefault
    (void)
{
    modifyFPUStateX86(__MOD_FPU_CW_DEFAULT__, __FPU_CW_MASK_ALL__);
    assertFPUModDefault();
}


inline
void
assertFPUModDefault
    (void)
{
    assert((getFPUStateX86() & (__FPU_CW_MASK_ALL__)) == 
            __MOD_FPU_CW_DEFAULT__);
}

// taken from http://www.stereopsis.com/FPU.html
// this assumes the CPU is in double precision mode
inline
int
FastFtol(const float a)
{
    static int    b;
    
#if defined(_MSVC)
    __asm fld a
    __asm fistp b
#elif defined(__GNUG__)
    // use AT&T inline assembly style, document that
    // we use memory as output (=m) and input (m)
    __asm__ __volatile__ (
        "flds %1        \n\t"
        "fistpl %0      \n\t"
        : "=m" (b)
        : "m" (a));
#endif
    return b;
}


#include <math.h>

inline int Floor2Int(float val)
{
	return (int)floorf(val);
}

inline int Ceil2Int(float val)
{
	return (int)ceilf(val);
}

inline int Round2Int(float val)
{
	return (int)floorf(val + 0.5f);
}

inline int Float2Int(float val)
{
	return (int)val;
}



#endif // FloatCast_h__
