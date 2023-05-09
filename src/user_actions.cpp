ACTION drop::openbal(
    name wallet,
    name contract,
    symbol symbol
) {
    require_auth(wallet);

    balances_t balances(get_self(), get_self().value);
    auto bal_itr = balances.find(wallet.value);

    if (bal_itr == balances.end()) {
        std::vector<extended_asset> tokens = {};
        tokens.push_back(extended_asset(asset(0, symbol), contract));
        balances.emplace(wallet, [&](auto &row) {
            row.wallet = wallet;
            row.tokens = tokens;
        });
    } else {
        std::vector<extended_asset> tokens = bal_itr->tokens;
        auto tokens_itr = std::find_if(
            tokens.begin(),
            tokens.end(),
            [&contract, &symbol](const extended_asset token) -> bool {
                return contract == token.contract && symbol == token.get_extended_symbol().get_symbol();
            }
        );
        if (tokens_itr != tokens.end()) {
            return;
        }
        tokens.push_back(extended_asset(asset(0, symbol), contract));
        balances.modify(bal_itr, same_payer, [&](auto &row) {
            row.tokens = tokens;
        });
    }
}

ACTION drop::closebal(
    name wallet
) {
    require_auth(wallet);

    balances_t balances(get_self(), get_self().value);
    auto bal_itr = balances.find(wallet.value);

    if (bal_itr != balances.end() && bal_itr->tokens.size() > 0) {
        std::vector<extended_asset> tokens = bal_itr->tokens;
        for (extended_asset token : tokens) {
            if (token.quantity.amount > 0) {
                transfer_token(wallet, token.contract, token.quantity, "Token balance claim");
            }
        }
        balances.erase(bal_itr);
    }
}

ACTION drop::claim(
    name wallet,
    uint64_t drop_id,
    uint32_t qty
) {
    require_auth(wallet);

    drops_t drops(get_self(), get_self().value);
    auto drop_itr = drops.find(drop_id);
    check(drop_itr != drops.end(), "Drop not found");

    if (drop_itr->start_time > 0) {
        check(current_time_point().sec_since_epoch() >= drop_itr->start_time, "Drop has not started");
    }
    if (drop_itr->end_time > 0) {
        check(current_time_point().sec_since_epoch() <= drop_itr->end_time, "Drop already ended");
    }
    if (drop_itr->max_claims > 0) {
        check((drop_itr->claims + qty) <= drop_itr->max_claims, "Drop has maxed out");
    }

    claims_t claims(get_self(), wallet.value);
    auto claim_itr = claims.find(drop_id);
    uint32_t claim = qty;
    if (drop_itr->limit > 0) {
        if (qty > drop_itr->limit) {
            check(false, ("Maximum of " + std::to_string(drop_itr->limit) + " per claim").c_str());
        }
        if (claim_itr != claims.end()) {
            if ((claim_itr->counter + qty) > drop_itr->limit) {
                check(current_time_point().sec_since_epoch() >= (claim_itr->last_claim + drop_itr->cooldown), "Please wait for cooldown");
            } else {
                claim = claim_itr->counter + qty;
            }
        }
    } else {
        if (claim_itr != claims.end()) {
            claim = claim_itr->counter + qty;
        }
    }

    extended_asset price = drop_itr->price;
    if (price.quantity.amount > 0) {
        balances_t balances(get_self(), get_self().value);
        auto bal_itr = balances.find(wallet.value);
        check(bal_itr != balances.end(), "Please run openbal action first");

        std::vector<extended_asset> tokens = bal_itr->tokens;
        auto token_itr = std::find_if(
            tokens.begin(),
            tokens.end(),
            [&price](const extended_asset token) -> bool {
                return token.contract == price.contract && token.get_extended_symbol().get_symbol() == price.get_extended_symbol().get_symbol();
            }
        );
        check(token_itr != tokens.end(), "No token balance");
        *token_itr -= extended_asset(asset(price.quantity.amount * qty, price.get_extended_symbol().get_symbol()), price.contract);
        check(token_itr->quantity.amount >= 0, "Insufficient balance");

        balances.modify(bal_itr, same_payer, [&](auto &row) {
            row.tokens = tokens;
        });
    }

    for (int32_t i = 0; i < qty; i++) {
        for (int32_t tmplt : drop_itr->target) {
            atomicassets::templates_t templates = atomicassets::get_templates(drop_itr->target_collection);
            auto tmplt_itr = templates.find(tmplt);
            mint_asset(drop_itr->target_collection, tmplt_itr->schema_name, tmplt, wallet);
        }
    }

    drops.modify(drop_itr, get_self(), [&](auto &row) {
        row.claims += qty;
    });

    if (claim_itr != claims.end()) {
        claims.modify(claim_itr, same_payer, [&](auto &row) {
            row.counter = claim;
            row.last_claim = current_time_point().sec_since_epoch();
        });
    } else {
        claims.emplace(wallet, [&](auto &row) {
            row.drop_id = drop_id;
            row.counter = claim;
            row.last_claim = current_time_point().sec_since_epoch();
        });
    }
}