// VeloxLA — src/backend/backend_dispatch.cpp
// Implementasi deteksi CPU vendor saat runtime. Dipisah jadi compiled TU
// (bukan header) untuk menghindari ODR issue karena di-include banyak tempat.
// TODO: implementasi cpuid-based vendor detection (Intel vs AMD).

#include <veloxla/backend/backend_dispatch.hpp>

namespace veloxla::backend {

}  // namespace veloxla::backend
