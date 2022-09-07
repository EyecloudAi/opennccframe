var native__vpu__api_8h =
[
    [ "NccUsbPortSpec_t", "struct_ncc_usb_port_spec__t.html", "struct_ncc_usb_port_spec__t" ],
    [ "NccPipeInput_t", "struct_ncc_pipe_input__t.html", "struct_ncc_pipe_input__t" ],
    [ "NccPipeOutput_t", "struct_ncc_pipe_output__t.html", "struct_ncc_pipe_output__t" ],
    [ "NccPipeHandle_t", "struct_ncc_pipe_handle__t.html", "struct_ncc_pipe_handle__t" ],
    [ "NccTensorSpec_t", "struct_ncc_tensor_spec__t.html", "struct_ncc_tensor_spec__t" ],
    [ "MAX_DEV_NUM", "native__vpu__api_8h.html#a9a1340a793f9b2a062707a22e2a63a1f", null ],
    [ "MAX_PIPELINE_NUM", "native__vpu__api_8h.html#a42fcea934d934da8f5a96e77bb223912", null ],
    [ "FRAMETYPE", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8", [
      [ "YUV420p", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8a8a1a60639c767912a6a02bd9efdd0073", null ],
      [ "YUV422p", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8a6c8763821324308c589a942d59b608b8", null ],
      [ "H26X", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8aad1bdea4eaf4cb50513fbe0cbecf3d70", null ],
      [ "JPEG", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8a96f4d8a8c2070cf7bdd2236a48eae07a", null ],
      [ "METEDATA", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8a4e300f646b90d30ed49ab2647fd2808a", null ],
      [ "BLOB", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8a3a625432a45f9c83112036bc7a2d2d70", null ],
      [ "BLOB_CFG", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8a67a040a25954b55d259094a4ba61d82d", null ],
      [ "NONE", "native__vpu__api_8h.html#a8fc2eb7158f7882e8b6ebf93d84023e8ac157bdf0b85a40d2619cbc8bc1ae5fe2", null ]
    ] ],
    [ "LOG_LEVEL", "native__vpu__api_8h.html#aa5a9053636a30269210c54e734e0d583", [
      [ "LOG_INFO", "native__vpu__api_8h.html#aa5a9053636a30269210c54e734e0d583a6e98ff471e3ce6c4ef2d75c37ee51837", null ],
      [ "LOG_DEBUG", "native__vpu__api_8h.html#aa5a9053636a30269210c54e734e0d583ab9f002c6ffbfd511da8090213227454e", null ],
      [ "LOG_ERROR", "native__vpu__api_8h.html#aa5a9053636a30269210c54e734e0d583a230506cce5c68c3bac5a821c42ed3473", null ],
      [ "LOG_SHOW", "native__vpu__api_8h.html#aa5a9053636a30269210c54e734e0d583a84b90241077b63cd8c67e7a375f5044e", null ]
    ] ],
    [ "METATYPE", "native__vpu__api_8h.html#a673fbe7956d763fafe9937e9d704d32f", [
      [ "META_FORMAT_U8", "native__vpu__api_8h.html#a673fbe7956d763fafe9937e9d704d32fa33716eac372dfee3e1262eb4a2818d4a", null ],
      [ "META_FORMAT_FP16", "native__vpu__api_8h.html#a673fbe7956d763fafe9937e9d704d32faba062836fc392866e73e9dffc16c1967", null ],
      [ "META_FORMAT_FP32", "native__vpu__api_8h.html#a673fbe7956d763fafe9937e9d704d32fae58fe8a8c1984e748e0b1e26687151ca", null ]
    ] ],
    [ "PROCESS_MODE", "native__vpu__api_8h.html#aa778cf399cedd1546dfff2e734ba49f4", [
      [ "NCC_SYNC", "native__vpu__api_8h.html#aa778cf399cedd1546dfff2e734ba49f4a06cd93014ebcbab9bb8e0aeecc339750", null ],
      [ "NCC_ASYNC", "native__vpu__api_8h.html#aa778cf399cedd1546dfff2e734ba49f4a2efbbf6bd2a5533187c79c97dfb47ab6", null ]
    ] ],
    [ "usb_error", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12b", [
      [ "USB_SUCCESS", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba68ee7000f7f5dbb4f98dfae7a80a4c0b", null ],
      [ "USB_ERROR_IO", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba960879789b1075bd9b47b979414d06aa", null ],
      [ "USB_ERROR_INVALID_PARAM", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba1ad3e7a09bb8449a427c858578cd5dfe", null ],
      [ "USB_ERROR_ACCESS", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12baa8316f49a013fe42c875b513f5b22f2c", null ],
      [ "USB_ERROR_NO_DEVICE", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba9c14d663c2bcafa5cd7b3d6af9f0d495", null ],
      [ "USB_ERROR_NOT_FOUND", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba6d2b8352d62751116ce7624b9586680f", null ],
      [ "USB_ERROR_BUSY", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba507e7f6e7fa2f1398d6e266c8ee57b12", null ],
      [ "USB_ERROR_TIMEOUT", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba85482063a7560c05eb83929f49605b11", null ],
      [ "USB_ERROR_OVERFLOW", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba440bda375171acc09bca52acec2a3d12", null ],
      [ "USB_ERROR_PIPE", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12baa09c38dc46f0e2ad78e798b2663325c8", null ],
      [ "USB_ERROR_INTERRUPTED", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12baa10e7fe7b5c6b315400c65a0effe63bb", null ],
      [ "USB_ERROR_NO_MEM", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12bac84412014e67c2f8ee3a08fb26447422", null ],
      [ "USB_ERROR_NOT_SUPPORTED", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12baea575ecc0c1db2c6f13e1f4cb69f7218", null ],
      [ "USB_ERROR_OTHER", "native__vpu__api_8h.html#a465873fe7681cf20279463acef99c12ba5498ddefadc3e75907fae6ed15e54dfc", null ]
    ] ],
    [ "async_process", "native__vpu__api_8h.html#a2d394e648e4e8fd44c64e2367f1b8032", null ],
    [ "ncc_dev_id_get", "native__vpu__api_8h.html#a004eb966cf02eb95c1fd1c00a62520e5", null ],
    [ "ncc_dev_init", "native__vpu__api_8h.html#a40381cfd4de56cecb4f7f33fd5d7f75b", null ],
    [ "ncc_dev_number_get", "native__vpu__api_8h.html#a86bb06155f887151f6489e6c34c30668", null ],
    [ "ncc_dev_serial_number_get", "native__vpu__api_8h.html#a4245d104471b2da36abc19468b1d9ccd", null ],
    [ "ncc_dev_start", "native__vpu__api_8h.html#a2a0b852128dbdbb54ad4e3d291d6eb29", null ],
    [ "ncc_free", "native__vpu__api_8h.html#a298a5120b438a6ef371fed08cf3a8c06", null ],
    [ "ncc_input_tensor_descriptor_get", "native__vpu__api_8h.html#a778addb3b33b06195f59c534fb80781a", null ],
    [ "ncc_malloc", "native__vpu__api_8h.html#a0791fdacea570763f5fd105358ce4096", null ],
    [ "ncc_output_tensor_descriptor_get", "native__vpu__api_8h.html#a6020d4eaab0ff40944ea43f24ebb4501", null ],
    [ "ncc_pipe_create", "native__vpu__api_8h.html#a59c755a40572ce3be0d24ff2f480aee4", null ],
    [ "ncc_pipe_id_get", "native__vpu__api_8h.html#ada492952f8dbaf720b09d195e04000b9", null ],
    [ "ncc_pipe_queue_read", "native__vpu__api_8h.html#a28a3cc175a88f0297a77bac718ef836e", null ],
    [ "set_log_level", "native__vpu__api_8h.html#a9a60358e3c4b0bc13aefac93fa07f5f5", null ],
    [ "sync_process", "native__vpu__api_8h.html#a2995f8739fbaf9efeda7393a7d8d342a", null ]
];