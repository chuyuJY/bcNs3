set(OPENSSL_LIBRARIES "-lssl -lcrypto")

build_lib(
  LIBNAME applications
  SOURCE_FILES
    helper/bulk-send-helper.cc
    helper/on-off-helper.cc
    helper/packet-sink-helper.cc
    helper/three-gpp-http-helper.cc
    helper/udp-client-server-helper.cc
    helper/udp-echo-helper.cc
    model/application-packet-probe.cc
    model/bulk-send-application.cc
    model/onoff-application.cc
    model/packet-loss-counter.cc
    model/packet-sink.cc
    model/seq-ts-echo-header.cc
    model/seq-ts-header.cc
    model/seq-ts-size-header.cc
    model/three-gpp-http-client.cc
    model/three-gpp-http-header.cc
    model/three-gpp-http-server.cc
    model/three-gpp-http-variables.cc
    model/udp-client.cc
    model/udp-echo-client.cc
    model/udp-echo-server.cc
    model/udp-server.cc
    model/udp-trace-client.cc
    helper/network-helper.cc
    model/pbft-node.cc
    model/block.cc
    model/message.cc
    model/crypto.cc
    
  HEADER_FILES
    helper/bulk-send-helper.h
    helper/on-off-helper.h
    helper/packet-sink-helper.h
    helper/three-gpp-http-helper.h
    helper/udp-client-server-helper.h
    helper/udp-echo-helper.h
    model/application-packet-probe.h
    model/bulk-send-application.h
    model/onoff-application.h
    model/packet-loss-counter.h
    model/packet-sink.h
    model/seq-ts-echo-header.h
    model/seq-ts-header.h
    model/seq-ts-size-header.h
    model/three-gpp-http-client.h
    model/three-gpp-http-header.h
    model/three-gpp-http-server.h
    model/three-gpp-http-variables.h
    model/udp-client.h
    model/udp-echo-client.h
    model/udp-echo-server.h
    model/udp-server.h
    model/udp-trace-client.h
    helper/network-helper.h
    model/pbft-node.h
    model/block.h
    model/message.h
    model/crypto.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libstats}
                    ${OPENSSL_LIBRARIES}
  TEST_SOURCES
    test/three-gpp-http-client-server-test.cc
    test/bulk-send-application-test-suite.cc
    test/udp-client-server-test.cc
)

# if (NOT EVENT__DISABLE_OPENSSL)
#     set(OPENSSL_USE_STATIC_LIBS TRUE)
#     set(OPENSSL_CRYPTO_LIBRARY "/home/zhangjiyuan/.local/openssl/lib")
#     set(OPENSSL_INCLUDE_DIR "/home/zhangjiyuan/.local/openssl/include")

#     find_package(OpenSSL REQUIRED)

#     set(EVENT__HAVE_OPENSSL 1)  

#     message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
#     message(STATUS "OpenSSL lib: ${OPENSSL_LIBRARIES}")

#     include_directories(${OPENSSL_INCLUDE_DIR})

#     list(APPEND SRC_OPENSSL bufferevent_openssl.c)
#     list(APPEND HDR_PUBLIC include/event2/bufferevent_ssl.h)
#     list(APPEND LIB_APPS ${OPENSSL_LIBRARIES})
# endif()
