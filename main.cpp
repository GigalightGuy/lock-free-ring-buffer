#include <atomic>
#include <iostream>
#include <string.h>
#include <thread>

struct ring_buffer {
  uint8_t *buffer;
  size_t size;
  std::atomic<size_t> writer_ptr;
  std::atomic<size_t> reader_ptr;
  std::atomic<size_t> watermark_ptr;
};

ring_buffer g_ring_buffer;
bool g_running = true;

void read() {
  std::string tmp_str;

  while (g_running) {
    if (g_ring_buffer.writer_ptr < g_ring_buffer.reader_ptr) {
      if (g_ring_buffer.reader_ptr < g_ring_buffer.watermark_ptr) {
        size_t str_size =
            g_ring_buffer.watermark_ptr - g_ring_buffer.reader_ptr;
        tmp_str.resize(str_size);
        memcpy((void *)tmp_str.c_str(),
               g_ring_buffer.buffer + g_ring_buffer.reader_ptr, str_size);
        g_ring_buffer.reader_ptr += str_size;
        std::cout << "Read: " << tmp_str << std::endl;
      } else {
        g_ring_buffer.reader_ptr = 0;
      }
    } else if (g_ring_buffer.reader_ptr < g_ring_buffer.writer_ptr) {
      size_t str_size = g_ring_buffer.writer_ptr - g_ring_buffer.reader_ptr;
      tmp_str.resize(str_size);
      memcpy((void *)tmp_str.c_str(),
             g_ring_buffer.buffer + g_ring_buffer.reader_ptr, str_size);
      g_ring_buffer.reader_ptr += str_size;
      std::cout << "Read: " << tmp_str << std::endl;
    }
  }
}

void write() {
  std::string tmp_str;
  bool ready_for_new_input = true;

  while (g_running) {
    if (ready_for_new_input) {
      std::cout << "Write: ";
      std::cin >> tmp_str;
      if (strcmp(tmp_str.c_str(), "exit") == 0) {
        g_running = false;
        continue;
      }
      ready_for_new_input = false;
    }

    if (g_ring_buffer.writer_ptr < g_ring_buffer.reader_ptr) {
      if (g_ring_buffer.writer_ptr <
          g_ring_buffer.reader_ptr - tmp_str.size()) {
        strcpy((char *)(g_ring_buffer.buffer + g_ring_buffer.writer_ptr),
               tmp_str.c_str());
        g_ring_buffer.writer_ptr += tmp_str.size();
        ready_for_new_input = true;
      }
    } else if (g_ring_buffer.writer_ptr <=
               g_ring_buffer.size - tmp_str.size()) {
      strcpy((char *)(g_ring_buffer.buffer + g_ring_buffer.writer_ptr),
             tmp_str.c_str());
      g_ring_buffer.writer_ptr += tmp_str.size();
      ready_for_new_input = true;
    } else {
      size_t prv_writer_ptr = g_ring_buffer.writer_ptr;
      g_ring_buffer.writer_ptr = 0;
      g_ring_buffer.watermark_ptr = prv_writer_ptr;
    }
  }
}

int main(int argc, char **argv) {

  g_ring_buffer.buffer = new uint8_t[1024 * 1];
  g_ring_buffer.size = 1024 * 1;
  g_ring_buffer.writer_ptr = 0;
  g_ring_buffer.reader_ptr = 0;
  g_ring_buffer.watermark_ptr = g_ring_buffer.size;

  std::thread reader(read);
  std::thread writer(write);

  writer.join();
  reader.join();

  return 0;
}
