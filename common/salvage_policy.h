#pragma once
#include "inventory_view.h"
#include <set>

namespace destinia {
constexpr size_t kMaxSalvageBatch = 50;

// Return stable inventory indices in reverse bag/slot order so earlier native
// removals cannot shift a later target. Equipped pools are supplied from the
// live Game equipment pointers; protection is carried by each item record.
inline std::vector<size_t> inventorySalvageCandidates(
    const std::vector<InventoryItem>& items,
    const std::vector<size_t>& view,
    const std::set<int>& equippedPools,
    size_t limit = kMaxSalvageBatch) {
    if (limit == 0 || limit > kMaxSalvageBatch)
        throw std::runtime_error("Limite de lote invalido.");
    std::set<size_t> seen;
    std::vector<size_t> candidates;
    for (const auto index : view) {
        if (index >= items.size() || !seen.insert(index).second)
            throw std::runtime_error("Visualizacao de inventario ambigua.");
        const auto& item = items[index];
        if (item.protectedItem || item.equipped || equippedPools.count(item.pool)) continue;
        candidates.push_back(index);
    }
    std::sort(candidates.begin(), candidates.end(), [&](size_t left, size_t right) {
        const auto& a = items[left];
        const auto& b = items[right];
        return std::pair<int, int>{a.bag, a.slot} > std::pair<int, int>{b.bag, b.slot};
    });
    if (candidates.size() > limit) candidates.resize(limit);
    return candidates;
}
}
