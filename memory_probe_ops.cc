#include <limits.h>
#include <atomic>
#include <vector>

#include "tensorflow/core/common_runtime/device.h"
#include "tensorflow/core/framework/device_base.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/framework/resource_mgr.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/refcount.h"
#include "tensorflow/core/lib/gtl/map_util.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/macros.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/thread_annotations.h"
#include "tensorflow/core/platform/types.h"

using namespace tensorflow;

REGISTER_OP("BytesInUse")
    .Output("output: int64")
    .SetIsStateful()
    .Doc(R"doc(Returns bytes in use for the device the op is placed on
    )doc");

class BytesInUseOp : public OpKernel {
 public:
  explicit BytesInUseOp(OpKernelConstruction* ctx) : OpKernel(ctx) {}

  void Compute(OpKernelContext* ctx) override {
    Tensor* output_tensor = NULL;
    OP_REQUIRES_OK(ctx, ctx->allocate_output(0, TensorShape({}),
                                             &output_tensor));
    auto output = output_tensor->flat<int64>();
    
    AllocatorAttributes alloc_attrs;
    auto device = static_cast<tensorflow::Device*>(ctx->device());
    Allocator* allocator = device->GetAllocator(alloc_attrs);
    // this doesn't do anything until cpu_allocator_collect_full_stats
    // is marked as extern
    EnableCPUAllocatorStats(true);
    AllocatorStats stats;
    allocator->GetStats(&stats);
    output(0) = stats.bytes_in_use;
  }
};

REGISTER_KERNEL_BUILDER(Name("BytesInUse").Device(DEVICE_CPU), BytesInUseOp);
REGISTER_KERNEL_BUILDER(Name("BytesInUse").Device(DEVICE_GPU).HostMemory("output"), BytesInUseOp);

REGISTER_OP("AllocatorName")
    .Output("output: string")
    .SetIsStateful()
    .Doc(R"doc(Returns name of current allocator
    )doc");

class AllocatorNameOp : public OpKernel {
 public:
  explicit AllocatorNameOp(OpKernelConstruction* ctx) : OpKernel(ctx) {}

  void Compute(OpKernelContext* ctx) override {
    Tensor* output_tensor = NULL;
    OP_REQUIRES_OK(ctx, ctx->allocate_output(0, TensorShape({}),
                                             &output_tensor));
    auto output = output_tensor->flat<string>();
    AllocatorAttributes alloc_attrs;
    auto device = static_cast<tensorflow::Device*>(ctx->device());
    Allocator* allocator = device->GetAllocator(alloc_attrs);
    output(0) = allocator->Name();
  }
};

REGISTER_KERNEL_BUILDER(Name("AllocatorName").Device(DEVICE_CPU), AllocatorNameOp);
REGISTER_KERNEL_BUILDER(Name("AllocatorName").Device(DEVICE_GPU).HostMemory("output"), AllocatorNameOp);

REGISTER_OP("BytesLimit")
    .Output("output: int64")
    .SetIsStateful()
    .Doc(R"doc(Returns bytes_limit for the device the op is placed on
    )doc");

class BytesLimitOp : public OpKernel {
 public:
  explicit BytesLimitOp(OpKernelConstruction* ctx) : OpKernel(ctx) {}

  void Compute(OpKernelContext* ctx) override {
    Tensor* output_tensor = NULL;
    OP_REQUIRES_OK(ctx, ctx->allocate_output(0, TensorShape({}),
                                             &output_tensor));
    auto output = output_tensor->flat<int64>();
    
    AllocatorAttributes alloc_attrs;
    auto device = static_cast<tensorflow::Device*>(ctx->device());
    Allocator* allocator = device->GetAllocator(alloc_attrs);
    AllocatorStats stats;
    allocator->GetStats(&stats);
    output(0) = stats.bytes_limit;
  }
};

REGISTER_KERNEL_BUILDER(Name("BytesLimit").Device(DEVICE_CPU), BytesLimitOp);
REGISTER_KERNEL_BUILDER(Name("BytesLimit").Device(DEVICE_GPU).HostMemory("output"), BytesLimitOp);
