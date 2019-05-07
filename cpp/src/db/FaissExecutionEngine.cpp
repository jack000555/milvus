/*******************************************************************************
 * Copyright 上海赜睿信息科技有限公司(Zilliz) - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 ******************************************************************************/
#ifndef FAISSEXECUTIONENGINE_CPP__
#define FAISSEXECUTIONENGINE_CPP__

#include <easylogging++.h>
#include <faiss/AutoTune.h>
#include <faiss/MetaIndexes.h>
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>
#include <wrapper/Index.h>
#include <wrapper/IndexBuilder.h>
#include <cache/CpuCacheMgr.h>

#include "FaissExecutionEngine.h"

namespace zilliz {
namespace vecwise {
namespace engine {


template<class IndexTrait>
FaissExecutionEngine<IndexTrait>::FaissExecutionEngine(uint16_t dimension, const std::string& location)
    : pIndex_(faiss::index_factory(dimension, IndexTrait::RawIndexType)),
      location_(location) {
}

template<class IndexTrait>
FaissExecutionEngine<IndexTrait>::FaissExecutionEngine(std::shared_ptr<faiss::Index> index, const std::string& location)
    : pIndex_(index),
      location_(location) {
}

template<class IndexTrait>
Status FaissExecutionEngine<IndexTrait>::AddWithIds(long n, const float *xdata, const long *xids) {
    pIndex_->add_with_ids(n, xdata, xids);
    return Status::OK();
}

template<class IndexTrait>
size_t FaissExecutionEngine<IndexTrait>::Count() const {
    return (size_t)(pIndex_->ntotal);
}

template<class IndexTrait>
size_t FaissExecutionEngine<IndexTrait>::Size() const {
    return (size_t)(Count() * pIndex_->d);
}

template<class IndexTrait>
size_t FaissExecutionEngine<IndexTrait>::PhysicalSize() const {
    return (size_t)(Size()*sizeof(float));
}

template<class IndexTrait>
Status FaissExecutionEngine<IndexTrait>::Serialize() {
    write_index(pIndex_.get(), location_.c_str());
    return Status::OK();
}

template<class IndexTrait>
Status FaissExecutionEngine<IndexTrait>::Load() {
    auto index  = zilliz::vecwise::cache::CpuCacheMgr::GetInstance()->GetIndex(location_);
    if (!index) {
        index = read_index(location_);
        Cache();
        LOG(DEBUG) << "Disk io from: " << location_;
    }

    pIndex_ = index->data();
    return Status::OK();
}

template<class IndexTrait>
Status FaissExecutionEngine<IndexTrait>::Merge(const std::string& location) {
    if (location == location_) {
        return Status::Error("Cannot Merge Self");
    }
    auto to_merge = zilliz::vecwise::cache::CpuCacheMgr::GetInstance()->GetIndex(location);
    if (!to_merge) {
        to_merge = read_index(location);
    }
    auto file_index = dynamic_cast<faiss::IndexIDMap*>(to_merge->data().get());
    pIndex_->add_with_ids(file_index->ntotal, dynamic_cast<faiss::IndexFlat*>(file_index->index)->xb.data(),
            file_index->id_map.data());
    return Status::OK();
}

template<class IndexTrait>
typename FaissExecutionEngine<IndexTrait>::Ptr
FaissExecutionEngine<IndexTrait>::BuildIndex(const std::string& location) {
    auto opd = std::make_shared<Operand>();
    opd->d = pIndex_->d;
    opd->index_type = IndexTrait::BuildIndexType;
    IndexBuilderPtr pBuilder = GetIndexBuilder(opd);

    auto from_index = dynamic_cast<faiss::IndexIDMap*>(pIndex_.get());

    auto index = pBuilder->build_all(from_index->ntotal,
            dynamic_cast<faiss::IndexFlat*>(from_index->index)->xb.data(),
            from_index->id_map.data());

    Ptr new_ee(new FaissExecutionEngine<IndexTrait>(index->data(), location));
    new_ee->Serialize();
    return new_ee;
}

template<class IndexTrait>
Status FaissExecutionEngine<IndexTrait>::Search(long n,
                                    const float *data,
                                    long k,
                                    float *distances,
                                    long *labels) const {

    pIndex_->search(n, data, k, distances, labels);
    return Status::OK();
}

template<class IndexTrait>
Status FaissExecutionEngine<IndexTrait>::Cache() {
    zilliz::vecwise::cache::CpuCacheMgr::GetInstance(
            )->InsertItem(location_, std::make_shared<Index>(pIndex_));

    return Status::OK();
}


} // namespace engine
} // namespace vecwise
} // namespace zilliz

#endif
