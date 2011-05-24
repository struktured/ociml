/* function related to AQ enqueue, listen and dequeue */

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <oci.h>
#include <ocidfn.h>
#include "oci_wrapper.h"

#define DEBUG

/* associate callback with datatype */
static struct custom_operations c_alloc_t_custom_ops = {
  "c_alloc_t_custom_ops", &caml_free_alloc_t, NULL, NULL, NULL, NULL}; 

/* get the TDO of the message_type and return a pointer to it in the OCI object cache */
value caml_oci_get_tdo(value env, value handles, value type_name) {
  CAMLparam3(env, handles, type_name);
  OCIEnv* e = Oci_env_val(env);
  oci_handles_t h = Oci_handles_val(handles);
  char* t = String_val(type_name);
#ifdef DEBUG
  char dbuf[256]; snprintf(dbuf, 255, "caml_oci_get_tdo: getting TDO for type '%s'", t); debug(dbuf);
#endif

  c_alloc_t tdo = {NULL};
  sword x = OCITypeByName(e, h.err, h.svc, NULL, 0, (text*)t, strlen(t), (text*)0, 0, OCI_DURATION_SESSION, OCI_TYPEGET_ALL, (OCIType**)&tdo.ptr);
  CHECK_OCI(x, h);

  value v = caml_alloc_custom(&c_alloc_t_custom_ops, sizeof(c_alloc_t), 0, 1);
  C_alloc_val(v) = tdo;

  CAMLreturn(v);
}

/* performs OCIStringAssignText() and returns a pointer to the memory */
value caml_oci_string_assign_text(value env, value handles, value str) {
  CAMLparam3(env, handles, str);
  OCIEnv* e = Oci_env_val(env);
  oci_handles_t h = Oci_handles_val(handles);
  char* s = String_val(str);
#ifdef DEBUG
  char dbuf[256]; snprintf(dbuf, 255, "caml_oci_string_assign_text: '%s'", s); debug(dbuf);
#endif
  c_alloc_t t = {NULL};

  sword x = OCIStringAssignText(e, h.err, (text*)s, strlen(s), (OCIString**)&t.ptr);
  CHECK_OCI(x, h);
  
#ifdef DEBUG
  snprintf(dbuf, 255, "caml_oci_string_assign_text: pointer is %p", t.ptr); debug(dbuf);
#endif

  value v = caml_alloc_custom(&c_alloc_t_custom_ops, sizeof(c_alloc_t), 0, 1);
  C_alloc_val(v) = t;

  CAMLreturn(v);
}

/* performs OCINumberFromInt() and returns a pointer to the memory */
value caml_oci_number_assign_int(value handles, value intnum) {
  CAMLparam2(handles, intnum);
  oci_handles_t h = Oci_handles_val(handles);
  int i = Int_val(intnum);
  sword x;

#ifdef DEBUG
  char dbuf[256]; snprintf(dbuf, 255, "caml_oci_number_assign_int: '%d'", i); debug(dbuf);
#endif
  c_alloc_t t = {NULL};

  /* malloc an OCINumber, write the int to it, and return a pointer to it */
  t.ptr = (OCINumber*)malloc(sizeof(OCINumber));
  x = OCINumberFromInt(h.err, &i, sizeof(int), OCI_NUMBER_SIGNED, (OCINumber*)&t.ptr);
  CHECK_OCI(x, h);

  value v = caml_alloc_custom(&c_alloc_t_custom_ops, sizeof(c_alloc_t), 0, 1);
  C_alloc_val(v) = t;
  
  CAMLreturn(v);
}

value caml_oci_int_from_number(value handles, value cht) {
  CAMLparam2(handles, cht);
  oci_handles_t h = Oci_handles_val(handles);
  c_alloc_t t = C_alloc_val(cht);

  int test;
  sword x = OCINumberToInt(h.err, (OCINumber*)&t.ptr, sizeof(int), OCI_NUMBER_SIGNED, &test);
  CHECK_OCI(x, h);
#ifdef DEBUG
  char dbuf[256]; snprintf(dbuf, 255, "caml_oci_int_from_number: retrieved number from pointer as %d", test); debug(dbuf);
#endif
  CAMLreturn(Val_int(test));
}

value caml_oci_string_from_string(value env, value cht) {
  CAMLparam2(env, cht);
  c_alloc_t t = C_alloc_val(cht);
  OCIEnv* e = Oci_env_val(env);
  CAMLreturn(caml_copy_string((char*)OCIStringPtr(e, t.ptr)));
}
/* actually enqueue the message */
value caml_oci_aq_enqueue(value handles, value queue_name, value message_tdo, value message, value null_message) {
  CAMLparam5(handles, queue_name, message_tdo, message, null_message);
  oci_handles_t h = Oci_handles_val(handles);
  char* qn = String_val(queue_name);
  c_alloc_t mt = C_alloc_val(message_tdo);
  c_alloc_t m = C_alloc_val(message);
  c_alloc_t nm = C_alloc_val(null_message);
#ifdef DEBUG
  char dbuf[256]; snprintf(dbuf, 255, "caml_oci_aq_enqueue: enqueueing message on '%s'",  qn); debug(dbuf);
#endif

    sword x = OCIAQEnq(h.svc, h.err, (text*)qn, 0, 0, mt.ptr, (dvoid**)&m.ptr, (dvoid**)&nm.ptr, 0, 0);
  CHECK_OCI(x, h);
#ifdef DEBUG
  debug("caml_oci_aq_enqueue: message enqueued successfully");
#endif
  CAMLreturn(Val_unit);
}
/* end of file */