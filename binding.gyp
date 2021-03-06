{
  "targets": [
    {
      "target_name": "packet_socket_addon",
      "sources": [ "src/wrapper.cc",
                   "src/socket.cc",
                   "src/poller.cc"],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")",
          "include"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ]
    },
    {
      "target_name": "test_wrapper",
      "sources": [ "src/wrapper.cc",
                   "cc_mocks/socket.cc",
                   "cc_mocks/poller.cc",
                   "cc_tests/wrapper.cc" ],
      "include_dirs": [
          "<!(node -e \"require('nan')\")",
          "include"
      ],
      "libraries": [ "-lCppUTest", "-lCppUTestExt"  ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ]
    },
    {
      "target_name": "test_poller",
      "sources": [ "src/poller.cc", "cc_tests/poller.cc" ],
      "include_dirs": [
          "<!(node -e \"require('nan')\")",
          "include"
      ],
      "libraries": [ "-lCppUTest", "-lCppUTestExt"  ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ]
    }
  ]
}
