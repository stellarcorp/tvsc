# Buffer

The buffer package contains abstractions for working with blocks of data, organized in different configurations.

## Buffer class

The main class, Buffer, handles a single block of data. It is very similar to the C++ type std::array, but features bulk read and write operations, as well as optimizations for [trivially copyable](https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable) types.

## RingBuffer class

The RingBuffer class handles a number of buffers logically organized into a ring. It allows for a single source to supply data while a single drain consumes the data. The RingBuffer provides a way to buffer both the source and sink sides of a stream of data while maintaining fixed memory usage.
