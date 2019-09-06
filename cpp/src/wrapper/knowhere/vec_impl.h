////////////////////////////////////////////////////////////////////////////////
// Copyright 上海赜睿信息科技有限公司(Zilliz) - All Rights Reserved
// Unauthorized copying of this file, via any medium is strictly prohibited.
// Proprietary and confidential.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "knowhere/index/vector_index/vector_index.h"

#include "vec_index.h"


namespace zilliz {
namespace milvus {
namespace engine {

class VecIndexImpl : public VecIndex {
 public:
    explicit VecIndexImpl(std::shared_ptr<zilliz::knowhere::VectorIndex> index, const IndexType &type)
        : index_(std::move(index)), type(type) {};
    ErrorCode BuildAll(const long &nb,
                                   const float *xb,
                                   const long *ids,
                                   const Config &cfg,
                                   const long &nt,
                                   const float *xt) override;
    VecIndexPtr CopyToGpu(const int64_t &device_id, const Config &cfg) override;
    VecIndexPtr CopyToCpu(const Config &cfg) override;
    IndexType GetType() override;
    int64_t Dimension() override;
    int64_t Count() override;
    ErrorCode Add(const long &nb, const float *xb, const long *ids, const Config &cfg) override;
    zilliz::knowhere::BinarySet Serialize() override;
    ErrorCode Load(const zilliz::knowhere::BinarySet &index_binary) override;
    VecIndexPtr Clone() override;
    int64_t GetDeviceId() override;
    ErrorCode Search(const long &nq, const float *xq, float *dist, long *ids, const Config &cfg) override;

 protected:
    int64_t dim = 0;
    IndexType type = IndexType::INVALID;
    std::shared_ptr<zilliz::knowhere::VectorIndex> index_ = nullptr;
};

class IVFMixIndex : public VecIndexImpl {
 public:
    explicit IVFMixIndex(std::shared_ptr<zilliz::knowhere::VectorIndex> index, const IndexType &type)
        : VecIndexImpl(std::move(index), type) {};

    ErrorCode BuildAll(const long &nb,
                                   const float *xb,
                                   const long *ids,
                                   const Config &cfg,
                                   const long &nt,
                                   const float *xt) override;
    ErrorCode Load(const zilliz::knowhere::BinarySet &index_binary) override;
};

class BFIndex : public VecIndexImpl {
 public:
    explicit BFIndex(std::shared_ptr<zilliz::knowhere::VectorIndex> index) : VecIndexImpl(std::move(index),
                                                                                          IndexType::FAISS_IDMAP) {};
    ErrorCode Build(const Config& cfg);
    float *GetRawVectors();
    ErrorCode BuildAll(const long &nb,
                                   const float *xb,
                                   const long *ids,
                                   const Config &cfg,
                                   const long &nt,
                                   const float *xt) override;
    int64_t *GetRawIds();
};

}
}
}
