{
  "targets": [
    {
      "target_name": "packet_socket_addon",
      "sources": [ "src/wrapper.cc" ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")",
          "include"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ]
    },
  ]
}

