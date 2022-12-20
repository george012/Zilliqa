const {expect} = require("chai");
const parallelizer = require("../helper/Parallelizer");

// TODO: Change the description to something more meaningful.
describe("Blockchain Instructions contract", function () {
  let contract;
  before(async function () {
    this.contract = await parallelizer.deployContract("BlockchainInstructions");
  });

  it("Should be deployed successfully", async function () {
    expect(this.contract.address).to.be.properAddress;
  });

  it("Should return the owner address when getOrigin function is called", async function () {
    const owner = this.contract.signer;
    expect(await this.contract.getOrigin()).to.be.eq(owner.address);
  });
});
