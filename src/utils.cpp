void drop::mint_asset(
    name collection,
    name schema,
    int32_t template_id,
    name receiver
) {
    std::vector<uint64_t> returning;
    atomicassets::ATTRIBUTE_MAP nodata = {};
    action(
        permission_level{get_self(), name("active")},
        atomicassets::ATOMICASSETS_ACCOUNT,
        name("mintasset"),
        std::make_tuple(
            get_self(),
            collection,
            schema,
            template_id,
            receiver,
            nodata,
            nodata,
            returning
        )
    ).send();
}

void drop::transfer_token(
    name receiver,
    name contract,
    asset amount,
    std::string memo
) {
    action(
        permission_level{get_self(), name("active")},
        contract,
        name("transfer"),
        std::make_tuple(
            get_self(),
            receiver,
            amount,
            memo
        )
    ).send();
}
