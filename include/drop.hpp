#include <eosio/eosio.hpp>
#include <atomicassets-interface.hpp>

using namespace eosio;

CONTRACT drop : public contract {
    public:
        using contract::contract;

        ACTION create(
            name target_collection,
            std::vector<int32_t> target,
            extended_asset price,
            uint32_t max_claims,
            uint32_t limit,
            uint32_t start_time,
            uint32_t end_time,
            uint32_t cooldown
        );

        ACTION updateclaims(
            uint64_t drop_id,
            uint32_t max_claims,
            uint32_t limit
        );

        ACTION updatetime(
            uint64_t drop_id,
            uint32_t start_time,
            uint32_t end_time,
            uint32_t cooldown
        );

        ACTION updateprice(
            uint64_t drop_id,
            extended_asset price
        );

        ACTION remove(
            uint64_t drop_id
        );

        ACTION openbal(
            name wallet,
            name contract,
            symbol symbol
        );

        ACTION closebal(
            name wallet
        );

        ACTION claim(
            name wallet,
            uint64_t drop_id,
            uint32_t qty
        );

        [[eosio::on_notify("*::transfer")]] void receive_token_transfer(
            name from,
            name to,
            asset quantity,
            std::string memo
        );

    private:
        TABLE drops_s {
            uint64_t drop_id;
            name target_collection;
            std::vector<int32_t> target;
            extended_asset price;
            uint32_t max_claims;
            uint32_t limit;
            uint32_t claims;
            uint32_t start_time;
            uint32_t end_time;
            uint32_t cooldown;

            auto primary_key() const { return drop_id; }
        };
        typedef multi_index<name("drops"), drops_s> drops_t;

        TABLE balances_s {
            name wallet;
            std::vector<extended_asset> tokens;

            auto primary_key() const { return wallet.value; }
        };
        typedef multi_index<name("balances"), balances_s> balances_t;

        TABLE claims_s {
            uint64_t drop_id;
            uint32_t counter;
            uint32_t last_claim;

            auto primary_key() const { return drop_id; }
        };
        typedef multi_index<name("claims"), claims_s> claims_t;

        void mint_asset(
            name collection,
            name schema,
            int32_t template_id,
            name receiver
        );

        void transfer_token(
            name receiver,
            name contract,
            asset amount,
            std::string memo
        );
};
