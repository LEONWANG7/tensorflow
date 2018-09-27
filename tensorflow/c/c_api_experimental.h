/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_C_C_API_EXPERIMENTAL_H_
#define TENSORFLOW_C_C_API_EXPERIMENTAL_H_

#include <stddef.h>
#include <stdint.h>

#include "tensorflow/c/c_api.h"
#include "tensorflow/c/eager/c_api.h"

// --------------------------------------------------------------------------
// Experimental C API for TensorFlow.
//
// The API here is subject to changes in the future.
// --------------------------------------------------------------------------

// Macro to control visibility of exported symbols in the shared library (.so,
// .dylib, .dll).
// This duplicates the TF_EXPORT macro definition in
// tensorflow/core/platform/macros.h in order to keep this .h file independent
// of any other includes.$a
#ifdef SWIG
#define TF_CAPI_EXPORT
#else
#if defined(_WIN32)
#ifdef TF_COMPILE_LIBRARY
#define TF_CAPI_EXPORT __declspec(dllexport)
#else
#define TF_CAPI_EXPORT __declspec(dllimport)
#endif  // TF_COMPILE_LIBRARY
#else
#define TF_CAPI_EXPORT __attribute__((visibility("default")))
#endif  // _WIN32
#endif  // SWIG

#ifdef __cplusplus
extern "C" {
#endif

// When `enable` is true, set
// tensorflow.ConfigProto.OptimizerOptions.global_jit_level to ON_1, and also
// set XLA flag values to prepare for XLA compilation. Otherwise set
// global_jit_level to OFF.
//
// This and the next API are syntax sugar over TF_SetConfig(), and is used by
// clients that cannot read/write the tensorflow.ConfigProto proto.
// TODO: Migrate to TF_CreateConfig() below.
TF_CAPI_EXPORT extern void TF_EnableXLACompilation(TF_SessionOptions* options,
                                                   unsigned char enable);

// Create a serialized tensorflow.ConfigProto proto, where:
//
// a) ConfigProto.optimizer_options.global_jit_level is set to to ON_1 if
// `enable_xla_compilation` is non-zero, and OFF otherwise.
// b) ConfigProto.gpu_options.allow_growth is set to `gpu_memory_allow_growth`.
TF_CAPI_EXPORT extern TF_Buffer* TF_CreateConfig(
    unsigned char enable_xla_compilation,
    unsigned char gpu_memory_allow_growth);

// Create a serialized tensorflow.RunOptions proto, where RunOptions.trace_level
// is set to FULL_TRACE if `enable_full_trace` is non-zero, and NO_TRACE
// otherwise.
TF_CAPI_EXPORT extern TF_Buffer* TF_CreateRunOptions(
    unsigned char enable_full_trace);

// Returns the graph content in a human-readable format, with length set in
// `len`. The format is subject to change in the future.
// The returned string is heap-allocated, and caller should call free() on it.
TF_CAPI_EXPORT extern const char* TF_GraphDebugString(TF_Graph* graph,
                                                      size_t* len);

// Creates a stack of data set + iterator nodes, currently hard-coded to return
// a sequence of 3 float values <42.0, 43.0, 44.0> over 3 calls. On success,
// returns the IteratorGetNext node, which caller can run or feed into an node.
//
// TODO(hongm): Extend the API to allow customization of the nodes created.
TF_CAPI_EXPORT extern TF_Operation* TF_MakeFakeIteratorGetNextWithDatasets(
    TF_Graph* graph, TF_Status* status);

// Similar to the above API, except that the returned iterator reads the
// file based dataset from `file_path`.
// If `is_mnist` is 0, the dataset corresponds to ImageNet.
// The iterators outputs 2 tensors:
// - A float tensor of shape `batch_size` X 784 when `is_mnist` is non-zero, or
// `batch_size` X 224 X 224 X 3 otherwise.
// - An int32 tensor of shape `batch_size`
// TODO(hongm): Extend the API to allow customization of the nodes created.
TF_CAPI_EXPORT extern TF_Operation* TF_MakeFileBasedIteratorGetNextWithDatasets(
    TF_Graph* graph, const char* file_path, int batch_size,
    unsigned char is_mnist, TF_Status* status);

// On success, dequeues a tensor from a TF-managed FifoQueue given by
// `tensor_id`, associated with `session`. There must be a graph node named
// "fifo_queue_dequeue_<tensor_id>", to be executed by this API call.

// Caller must call TF_DeleteTensor() over the returned tensor. If the queue is
// empty, this call is blocked.
//
// Tensors are enqueued via the corresponding TF enqueue op.
// TODO(hongm): Add support for `timeout_ms`.
TF_CAPI_EXPORT extern TF_Tensor* TF_DequeueNamedTensor(TF_Session* session,
                                                       int tensor_id,
                                                       TF_Status* status);

// On success, enqueues `tensor` into a TF-managed FifoQueue given by
// `tensor_id`, associated with `session`. There must be a graph node named
// "fifo_queue_enqueue_<tensor_id>", to be executed by this API call. It reads
// from a placeholder node "arg_tensor_enqueue_<tensor_id>".
//
// `tensor` is still owned by the caller. This call will be blocked if the queue
// has reached its capacity, and will be unblocked when the queued tensors again
// drop below the capacity due to dequeuing.
//
// Tensors are dequeued via the corresponding TF dequeue op.
// TODO(hongm): Add support for `timeout_ms`.
TF_CAPI_EXPORT extern void TF_EnqueueNamedTensor(TF_Session* session,
                                                 int tensor_id,
                                                 TF_Tensor* tensor,
                                                 TF_Status* status);
// Create a serialized tensorflow.ServerDef proto.
TF_Buffer* TFE_GetServerDef(const char* text_proto, TF_Status* status);

// TODO: remove this API in favor of the next one.
TF_CAPI_EXPORT extern TFE_Context* TFE_NewContextFromSession(
    const TFE_ContextOptions* opts, TF_Session* sess, TF_Status* status);

// Creates from `session` a new eager context to run a graph function or
// sends/recvs, so that these concurrent TFE executions can share (via
// `session` and its associated device mgr) the same set of fifo queue resource
// ops, used for host<->TF tensor transfers. This way the sends/recvs calls and
// graph function execution can access the same fifo queue resource handles
// (associated with devices managed by the device manager, which can be obtained
// from `session`).
//
// TODO: Remove this function once we migrate away from using session.
TF_CAPI_EXPORT extern TFE_Context* TFE_CreateContextFromSession(
    TF_Session* session, TF_Status* status);

// TODO: Retire this API in favor of the next one.
TF_CAPI_EXPORT extern TFE_TensorHandle* TFE_DequeueNamedTensor(
    TF_Session* session, int tensor_id, TF_DataType inputType,
    TF_Status* status);

TF_CAPI_EXPORT extern TFE_TensorHandle* TFE_DequeueNamedTensorFromCtx(
    TFE_Context* ctx, int tensor_id, TF_DataType inputType, TF_Status* status);

TF_CAPI_EXPORT extern void TFE_EnqueueNamedTensor(TF_Session* session,
                                                  int tensor_id,
                                                  TFE_TensorHandle* tensor,
                                                  TF_Status* status);

TF_CAPI_EXPORT extern void TFE_EnqueueNamedTensorFromCtx(
    TFE_Context* ctx, int tensor_id, TFE_TensorHandle* tensor,
    TF_Status* status);

// TODO: consider folding the 2 APIs below into the ones above.
TF_CAPI_EXPORT extern void TFE_EnqueueVariantTensor(TF_Session* session,
                                                    int tensor_id,
                                                    TFE_TensorHandle* tensor,
                                                    TF_Status* status);

TF_CAPI_EXPORT extern TFE_TensorHandle* TFE_DequeueVariantTensor(
    TF_Session* session, int tensor_id, TF_Status* status);

// Prints `handle` in a human readable format to standard output for debugging.
TF_CAPI_EXPORT extern void TFE_TensorHandlePrintDebugString(
    TFE_TensorHandle* handle);

// Returns a const scalar tensor.
// Caller owns both the input and the output tensor handles.
// TODO: Remove this API with hard-coded tensor computation.
TF_CAPI_EXPORT extern TFE_TensorHandle* TFE_RunConstOp(TFE_Context* ctx);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_C_API_EXPERIMENTAL_H_
