#include <drop.hpp>
#include "contract_actions.cpp"
#include "user_actions.cpp"
#include "utils.cpp"

void drop::receive_token_transfer(
    name from,
    name to,
    asset quantity,
    std::string memo
) {
    if (to != get_self()) {
        return;
    }

    balances_t balances(get_self(), get_self().value);
    auto bal_itr = balances.find(from.value);
    check(bal_itr != balances.end(), "Please run openbal action first");

    std::vector<extended_asset> tokens = bal_itr->tokens;
    name contract = get_first_receiver();
    auto token_itr = std::find_if(
        tokens.begin(),
        tokens.end(),
        [&contract, &quantity](const extended_asset token) -> bool {
            return contract == token.contract && quantity.symbol == token.get_extended_symbol().get_symbol();
        }
    );
    check(token_itr != tokens.end(), "Please run openbal action first");
    *token_itr += extended_asset(quantity, get_first_receiver());

    balances.modify(bal_itr, same_payer, [&](auto &row) {
        row.tokens = tokens;
    });
}