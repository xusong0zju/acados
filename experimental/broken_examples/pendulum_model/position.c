/* This file was automatically generated by CasADi.
   The CasADi copyright holders make no ownership claim of its contents. */
#ifdef __cplusplus
extern "C" {
#endif

/* How to prefix internal symbols */
#ifdef CODEGEN_PREFIX
  #define NAMESPACE_CONCAT(NS, ID) _NAMESPACE_CONCAT(NS, ID)
  #define _NAMESPACE_CONCAT(NS, ID) NS ## ID
  #define CASADI_PREFIX(ID) NAMESPACE_CONCAT(CODEGEN_PREFIX, ID)
#else
  #define CASADI_PREFIX(ID) position_ ## ID
#endif

#include <math.h>

#ifndef casadi_real
#define casadi_real double
#endif

#ifndef casadi_int
#define casadi_int int
#endif

/* Add prefix to internal symbols */
#define casadi_f0 CASADI_PREFIX(f0)
#define casadi_s0 CASADI_PREFIX(s0)
#define casadi_s1 CASADI_PREFIX(s1)
#define casadi_s2 CASADI_PREFIX(s2)

static const casadi_int casadi_s0[8] = {4, 1, 0, 4, 0, 1, 2, 3};
static const casadi_int casadi_s1[6] = {2, 1, 0, 2, 0, 1};
static const casadi_int casadi_s2[8] = {4, 2, 0, 2, 3, 0, 2, 2};

/* position:(i0[4])->(o0[2],o1[4x2,3nz]) */
static int casadi_f0(const casadi_real** arg, casadi_real** res, casadi_int* iw, casadi_real* w, void* mem) {
  casadi_real a0, a1, a2, a3;
  a0=arg[0] ? arg[0][0] : 0;
  a1=8.0000000000000004e-01;
  a2=arg[0] ? arg[0][2] : 0;
  a3=sin(a2);
  a3=(a1*a3);
  a0=(a0-a3);
  a0=(a0-a1);
  if (res[0]!=0) res[0][0]=a0;
  a0=cos(a2);
  a0=(a1*a0);
  a0=(a0-a1);
  if (res[0]!=0) res[0][1]=a0;
  a0=1.;
  if (res[1]!=0) res[1][0]=a0;
  a0=cos(a2);
  a0=(a1*a0);
  a0=(-a0);
  if (res[1]!=0) res[1][1]=a0;
  a2=sin(a2);
  a1=(a1*a2);
  a1=(-a1);
  if (res[1]!=0) res[1][2]=a1;
  return 0;
}

int position(const casadi_real** arg, casadi_real** res, casadi_int* iw, casadi_real* w, void* mem){
  return casadi_f0(arg, res, iw, w, mem);
}

void position_incref(void) {
}

void position_decref(void) {
}

casadi_int position_n_in(void) { return 1;}

casadi_int position_n_out(void) { return 2;}

const char* position_name_in(casadi_int i){
  switch (i) {
    case 0: return "i0";
    default: return 0;
  }
}

const char* position_name_out(casadi_int i){
  switch (i) {
    case 0: return "o0";
    case 1: return "o1";
    default: return 0;
  }
}

const casadi_int* position_sparsity_in(casadi_int i) {
  switch (i) {
    case 0: return casadi_s0;
    default: return 0;
  }
}

const casadi_int* position_sparsity_out(casadi_int i) {
  switch (i) {
    case 0: return casadi_s1;
    case 1: return casadi_s2;
    default: return 0;
  }
}

int position_work(casadi_int *sz_arg, casadi_int* sz_res, casadi_int *sz_iw, casadi_int *sz_w) {
  if (sz_arg) *sz_arg = 1;
  if (sz_res) *sz_res = 2;
  if (sz_iw) *sz_iw = 0;
  if (sz_w) *sz_w = 0;
  return 0;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
