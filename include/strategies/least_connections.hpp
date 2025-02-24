#include "strategies/strategy.hpp"
#include "core/server.hpp"
#include <vector>
#include <memory>

class LeastConnectionsStrategy : public Strategy {
public:
    std::shared_ptr<Server> selectServer(const std::vector<std::shared_ptr<Server>>& servers,
                                         const loadbalancer::Request& request) override;
};