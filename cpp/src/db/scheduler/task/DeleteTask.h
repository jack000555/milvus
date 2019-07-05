/*******************************************************************************
 * Copyright 上海赜睿信息科技有限公司(Zilliz) - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 ******************************************************************************/
#pragma once

#include "IScheduleTask.h"
#include "db/scheduler/context/DeleteContext.h"

namespace zilliz {
namespace milvus {
namespace engine {

class DeleteTask : public IScheduleTask {
public:
    DeleteTask(const DeleteContextPtr& context);

    virtual std::shared_ptr<IScheduleTask> Execute() override;

private:
    DeleteContextPtr context_;
};

using DeleteTaskPtr = std::shared_ptr<DeleteTask>;

}
}
}
