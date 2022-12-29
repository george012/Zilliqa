/*
 * Copyright (C) 2022 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ZILLIQA_SRC_LIBDATA_ACCOUNTDATA_ACCOUNTSTORECPSINTERFACE_H_
#define ZILLIQA_SRC_LIBDATA_ACCOUNTDATA_ACCOUNTSTORECPSINTERFACE_H_

#include "libData/AccountData/AccountStoreSC.h"

#include "libCps/CpsAccountStoreInterface.h"

#include "libPersistence/ContractStorage.h"

template <typename T>
struct AccountStoreCpsInterface : public libCps::CpsAccountStoreInterface {
 public:
  explicit AccountStoreCpsInterface(AccountStoreSC<T>& accStore)
      : mAccountStore(accStore) {}
  virtual libCps::Amount GetBalanceForAccountAtomic(
      const Address& address) override {
    const Account* account = mAccountStore.GetAccountAtomic(address);
    if (account != nullptr) {
      return libCps::Amount::fromQa(account->GetBalance());
    }
    return libCps::Amount{};
  }

  virtual uint64_t GetNonceForAccount(const Address& account) override {
    return mAccountStore.GetNonce(account);
  }

  virtual void SetNonceForAccount(const Address& address,
                                  uint64_t nonce) override {
    Account* account = mAccountStore.GetAccount(address);
    if (account != nullptr) {
      account->SetNonce(nonce);
    }
  }

  virtual bool AccountExists(const Address& account) override {
    return mAccountStore.GetAccount(account) != nullptr;
  }
  virtual bool AccountExistsAtomic(const Address& address) override {
    return mAccountStore.GetAccountAtomic(address) != nullptr;
  }
  virtual bool AddAccountAtomic(const Address& address) override {
    if (!mAccountStore.AddAccountAtomic(address, {0, 0})) {
      return false;
    }
    return true;
  }
  virtual Address GetAddressForContract(const Address& account,
                                        uint32_t transaction_version) override {
    return Account::GetAddressForContract(account, GetNonceForAccount(account),
                                          transaction_version);
  }
  virtual bool IncreaseBalance(const Address& account,
                               libCps::Amount amount) override {
    return mAccountStore.IncreaseBalance(account, amount.toQa());
  }
  virtual bool DecreaseBalance(const Address& account,
                               libCps::Amount amount) override {
    return mAccountStore.DecreaseBalance(account, amount.toQa());
  }
  virtual void SetBalanceAtomic(const Address& address,
                                libCps::Amount amount) override {
    Account* account = mAccountStore.GetAccountAtomic(address);
    if (account != nullptr) {
      account->SetBalance(amount.toQa());
    }
  }

  virtual bool TransferBalanceAtomic(const Address& from, const Address& to,
                                     libCps::Amount amount) override {
    return mAccountStore.TransferBalanceAtomic(from, to, amount.toQa());
  }

  virtual void DiscardAtomics() override { mAccountStore.DiscardAtomics(); }
  virtual void CommitAtomics() override { mAccountStore.CommitAtomics(); }

  virtual bool UpdateStates(const Address& address,
                            const std::map<std::string, zbytes>& states,
                            const std::vector<std::string>& toDeleteIndices,
                            bool temp, bool revertible = false) override {
    Account* account = mAccountStore.GetAccountAtomic(address);
    if (account != nullptr) {
      return account->UpdateStates(address, states, toDeleteIndices, temp,
                                   revertible);
    }
    return false;
  }

  virtual bool UpdateStateValue(const Address& address, const zbytes& q,
                                unsigned int q_offset, const zbytes& v,
                                unsigned int v_offset) override {
    return Contract::ContractStorage::GetContractStorage().UpdateStateValue(
        address, q, q_offset, v, v_offset);
  }

  virtual void AddAddressToUpdateBufferAtomic(const Address& addr) override {
    mAccountStore.m_storageRootUpdateBufferAtomic.emplace(addr);
  }

  virtual void SetImmutableAtoimic(const Address& address, const zbytes& code,
                                   const zbytes& initData) override {
    Account* account = mAccountStore.GetAccountAtomic(address);
    if (account != nullptr) {
      account->SetImmutable(code, initData);
    }
  }

  virtual void SetNonceForAccountAtomic(const Address& address,
                                        uint64_t nonce) override {
    Account* account = mAccountStore.GetAccountAtomic(address);
    if (account != nullptr) {
      account->SetNonce(nonce);
    }
  }

  virtual uint64_t GetNonceForAccountAtomic(const Address& address) override {
    Account* account = mAccountStore.GetAccountAtomic(address);
    if (account != nullptr) {
      account->GetNonce();
    }
    return 0;
  }

  virtual void FetchStateDataForContract(
      std::map<std::string, zbytes>& states, const dev::h160& address,
      const std::string& vname, const std::vector<std::string>& indices,
      bool temp) override {
    Contract::ContractStorage::GetContractStorage().FetchStateDataForContract(
        states, address, vname, indices, temp);
  }

 private:
  AccountStoreSC<T>& mAccountStore;
};

#endif /* ZILLIQA_SRC_LIBDATA_ACCOUNTDATA_ACCOUNTSTORECPSINTERFACE_H_ */