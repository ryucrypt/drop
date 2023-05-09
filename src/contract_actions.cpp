ACTION drop::create(
    name target_collection,
    std::vector<int32_t> target,
    extended_asset price,
    uint32_t max_claims,
    uint32_t limit,
    uint32_t start_time,
    uint32_t end_time,
    uint32_t cooldown
) {
    require_auth(get_self());

    if (atomicassets::collections.find(target_collection.value) == atomicassets::collections.end())
        check(false, (target_collection.to_string() + " does not exist").c_str());

    atomicassets::templates_t templates = atomicassets::get_templates(target_collection);
    for (int32_t i : target) {
        if (templates.find(i) == templates.end()) {
            check(false, ("Template " + std::to_string(i) + " not found in " + target_collection.to_string()).c_str());
        }
    }

    drops_t drops(get_self(), get_self().value);
    drops.emplace(get_self(), [&](auto &row) {
        row.drop_id = drops.available_primary_key();
        row.target_collection = target_collection;
        row.target = target;
        row.price = price;
        row.max_claims = max_claims;
        row.limit = limit;
        row.claims = 0;
        row.start_time = start_time;
        row.end_time = end_time;
        row.cooldown = cooldown;
    });
}

ACTION drop::updateclaims(
    uint64_t drop_id,
    uint32_t max_claims,
    uint32_t limit
) {
    require_auth(get_self());

    drops_t drops(get_self(), get_self().value);
    auto drop_itr = drops.find(drop_id);
    check(drop_itr != drops.end(), "Record does not exist");
    drops.modify(drop_itr, get_self(), [&](auto &row) {
        row.max_claims = max_claims;
        row.limit = limit;
    });
}

ACTION drop::updatetime(
    uint64_t drop_id,
    uint32_t start_time,
    uint32_t end_time,
    uint32_t cooldown
) {
    require_auth(get_self());

    drops_t drops(get_self(), get_self().value);
    auto drop_itr = drops.find(drop_id);
    check(drop_itr != drops.end(), "Record does not exist");
    drops.modify(drop_itr, get_self(), [&](auto &row) {
        row.start_time = start_time;
        row.end_time = end_time;
        row.cooldown  = cooldown;
    });
}

ACTION drop::updateprice(
    uint64_t drop_id,
    extended_asset price
) {
    require_auth(get_self());

    drops_t drops(get_self(), get_self().value);
    auto drop_itr = drops.find(drop_id);
    check(drop_itr != drops.end(), "Record does not exist");
    drops.modify(drop_itr, get_self(), [&](auto &row) {
        row.price = price;
    });
}

ACTION drop::remove(
    uint64_t drop_id
) {
    require_auth(get_self());

    drops_t drops(get_self(), get_self().value);
    auto drop_itr = drops.find(drop_id);
    check(drop_itr != drops.end(), "Record does not exist");
    drops.erase(drop_itr);
}
