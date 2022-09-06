require("@nomicfoundation/hardhat-toolbox");
require("@nomiclabs/hardhat-web3");

/** @type import('hardhat/config').HardhatUserConfig */
module.exports = {
  solidity: "0.8.9",
  defaultNetwork: "ganache",
  networks: {
    ganache: {
      url: "http://127.0.0.1:7545",
      chainId: 1337,
      accounts: [
        "c95690aed4461afd835b17492ff889af72267a8bdf7d781e305576cd8f7eb182",
        "05751249685e856287c2b2b9346e70a70e1d750bc69a35cef740f409ad0264ad"
      ],
      zilliqaNetwork: false,
    },
    zilliqa: {
      url: "https://master-mod-api.dev.z7a.xyz/",
      accounts: ["d96e9eb5b782a80ea153c937fa83e5948485fbfc8b7e7c069d7b914dbc350aba"],
      chainId: 33101,
      zilliqaNetwork: true,
    },
    isolated_server: {
      url: "http://localhost:5555/",
      accounts: ["d96e9eb5b782a80ea153c937fa83e5948485fbfc8b7e7c069d7b914dbc350aba",
                 "5c497e53404aca018264588643a3b6b48c6a03579da1538628acf827c4a264b3"
       ],
      chainId: 0x8001,
      zilliqaNetwork: true,
    }
  }
};

extendEnvironment((hre) => {
  // Returns true whenever active network is zilliqa related, o.w it returns false (e.g. for ganache)
  hre.isZilliqaNetworkSelected = function () {
    return hre.network.config.zilliqaNetwork;
  }

  hre.getEthChainId = function () {
    return hre.network.config.chainId;
  }

  hre.getZilliqaChainId = function () {
    return hre.network.config.chainId - 0x8000;
  }

  hre.getNetworkUrl = function () {
    return hre.network.config.url
  }

  hre.getPrivateAddressAt = function (index) {
    return hre.network.config.accounts[index]
  }
});