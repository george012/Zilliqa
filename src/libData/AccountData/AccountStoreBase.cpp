/*
 * Copyright (C) 2019 Zilliqa
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

#include <type_traits>

#include "libData/AccountData/AccountStoreBase.h"
#include "libMessage/MessengerAccountStoreBase.h"
#include "libUtils/Logger.h"
#include "libUtils/SafeMath.h"

void AccountStoreBase::Init() { m_addressToAccount.clear(); }

bool AccountStoreBase::Serialize(bytes& dst, unsigned int offset) const {
  if (!MessengerAccountStoreBase::SetAccountStore(dst, offset,
                                                  m_addressToAccount)) {
    LOG_GENERAL(WARNING, "Messenger::SetAccountStore failed.");
    return false;
  }

  return true;
}

bool AccountStoreBase::Deserialize(const bytes& src, unsigned int offset) {
  if (!MessengerAccountStoreBase::GetAccountStore(src, offset,
                                                  m_addressToAccount)) {
    LOG_GENERAL(WARNING, "Messenger::GetAccountStore failed.");
    return false;
  }

  return true;
}

bool AccountStoreBase::Deserialize(const std::string& src,
                                   unsigned int offset) {
  if (!MessengerAccountStoreBase::GetAccountStore(src, offset,
                                                  m_addressToAccount)) {
    LOG_GENERAL(WARNING, "Messenger::GetAccountStore failed.");
    return false;
  }

  return true;
}

bool AccountStoreBase::UpdateBaseAccounts(const Transaction& transaction,
                                          TransactionReceipt& receipt,
                                          TxnStatus& error_code) {
  const PubKey& senderPubKey = transaction.GetSenderPubKey();
  const Address fromAddr = Account::GetAddressFromPublicKey(senderPubKey);
  Address toAddr = transaction.GetToAddr();
  const uint128_t& amount = transaction.GetAmount();
  error_code = TxnStatus::NOT_PRESENT;

  Account* fromAccount = this->GetAccount(fromAddr);
  if (fromAccount == nullptr) {
    LOG_GENERAL(WARNING, "sender " << fromAddr.hex() << " not exist");
    error_code = TxnStatus::INVALID_FROM_ACCOUNT;
    return false;
  }

  if (transaction.GetGasLimit() < NORMAL_TRAN_GAS) {
    LOG_GENERAL(WARNING,
                "The gas limit "
                    << transaction.GetGasLimit()
                    << " should be larger than the normal transaction gas ("
                    << NORMAL_TRAN_GAS << ")");
    error_code = TxnStatus::INSUFFICIENT_GAS_LIMIT;
    return false;
  }

  uint128_t gasDeposit = 0;
  if (!SafeMath<uint128_t>::mul(transaction.GetGasLimit(),
                                transaction.GetGasPrice(), gasDeposit)) {
    LOG_GENERAL(
        WARNING,
        "transaction.GetGasLimit() * transaction.GetGasPrice() overflow!");
    error_code = TxnStatus::MATH_ERROR;
    return false;
  }

  if (fromAccount->GetBalance() < transaction.GetAmount() + gasDeposit) {
    LOG_GENERAL(WARNING,
                "The account (balance: "
                    << fromAccount->GetBalance()
                    << ") "
                       "doesn't have enough balance to pay for the gas limit ("
                    << gasDeposit
                    << ") "
                       "with amount ("
                    << transaction.GetAmount() << ") in the transaction");
    error_code = TxnStatus::INSUFFICIENT_BALANCE;
    return false;
  }

  if (!DecreaseBalance(fromAddr, gasDeposit)) {
    error_code = TxnStatus::MATH_ERROR;
    return false;
  }

  if (!TransferBalance(fromAddr, toAddr, amount)) {
    if (!IncreaseBalance(fromAddr, gasDeposit)) {
      LOG_GENERAL(FATAL, "IncreaseBalance failed for gasDeposit");
    }
    error_code = TxnStatus::MATH_ERROR;
    return false;
  }

  uint128_t gasRefund;
  if (!CalculateGasRefund(gasDeposit, NORMAL_TRAN_GAS,
                          transaction.GetGasPrice(), gasRefund)) {
    error_code = TxnStatus::MATH_ERROR;
    return false;
  }

  if (!IncreaseBalance(fromAddr, gasRefund)) {
    error_code = TxnStatus::MATH_ERROR;
    LOG_GENERAL(FATAL, "IncreaseBalance failed for gasRefund");
  }

  if (!IncreaseNonce(fromAddr)) {
    error_code = TxnStatus::MATH_ERROR;
    return false;
  }

  receipt.SetResult(true);
  receipt.SetCumGas(NORMAL_TRAN_GAS);
  receipt.update();

  return true;
}

bool AccountStoreBase::CalculateGasRefund(const uint128_t& gasDeposit,
                                          const uint64_t& gasUnit,
                                          const uint128_t& gasPrice,
                                          uint128_t& gasRefund) {
  uint128_t gasFee;
  if (!SafeMath<uint128_t>::mul(gasUnit, gasPrice, gasFee)) {
    LOG_GENERAL(WARNING, "gasUnit * transaction.GetGasPrice() overflow!");
    return false;
  }

  if (!SafeMath<uint128_t>::sub(gasDeposit, gasFee, gasRefund)) {
    LOG_GENERAL(WARNING, "gasDeposit - gasFee overflow!");
    return false;
  }

  // LOG_GENERAL(INFO, "gas price to refund: " << gasRefund);
  return true;
}

bool AccountStoreBase::IsAccountExist(const Address& address) {
  // LOG_MARKER();
  return (nullptr != GetAccount(address));
}

bool AccountStoreBase::AddAccount(const Address& address,
                                  const Account& account, bool toReplace) {
  // LOG_MARKER();
  if (toReplace || !IsAccountExist(address)) {
    m_addressToAccount[address] = account;

    return true;
  }
  LOG_GENERAL(WARNING, "Address "
                           << address
                           << " could not be added because already present");
  return false;
}

bool AccountStoreBase::AddAccount(const PubKey& pubKey,
                                  const Account& account) {
  return AddAccount(Account::GetAddressFromPublicKey(pubKey), account);
}

void AccountStoreBase::RemoveAccount(const Address& address) {
    m_addressToAccount.erase(address);
}

Account* AccountStoreBase::GetAccount(const Address& address) {
  auto it = m_addressToAccount.find(address);
  if (it != m_addressToAccount.end()) {
    return &it->second;
  }
  return nullptr;
}

const Account* AccountStoreBase::GetAccount(const Address& address) const {
  auto it = m_addressToAccount.find(address);
  if (it != m_addressToAccount.end()) {
    return &it->second;
  }
  return nullptr;
}

void AccountStoreBase::PrintAccountState() {
  LOG_MARKER();
  for (const auto& entry : m_addressToAccount) {
    LOG_GENERAL(INFO, entry.first << " " << entry.second);
  }
}
