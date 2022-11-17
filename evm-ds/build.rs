extern crate protoc_rust;

fn main() {
    protoc_rust::Codegen::new()
        .out_dir("src/protos")
        .inputs(&["../Zilliqa/src/libPersistence/ScillaMessage.proto"])
        .include("../Zilliqa/src/libPersistence")
        .customize(protoc_rust::Customize {
            carllerche_bytes_for_bytes: Some(true),
            carllerche_bytes_for_string: Some(true),
            ..Default::default()
        })
        .run()
        .expect("Running protoc failed for ScillaMessage.proto");

    protoc_rust::Codegen::new()
        .out_dir("src/protos")
        .inputs(&["../Zilliqa/src/libUtils/Evm.proto"])
        .include("../Zilliqa/src/libUtils")
        .customize(protoc_rust::Customize {
            carllerche_bytes_for_bytes: Some(true),
            carllerche_bytes_for_string: Some(true),
            ..Default::default()
        })
        .run()
        .expect("Running protoc failed for EVM.proto");

    println!("cargo:rerun-if-changed=../Zilliqa/src/libPersistence/ScillaMessage.proto");
    println!("cargo:rerun-if-changed=../Zilliqa/src/libUtils/Evm.proto");
}
