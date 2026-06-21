#pragma once
// VeloxLA — single include umbrella header
//
// #include <veloxla/veloxla.hpp> untuk akses seluruh API publik.

#include <veloxla/core/tensor.hpp>
#include <veloxla/core/storage.hpp>
#include <veloxla/core/shape.hpp>
#include <veloxla/core/layout.hpp>
#include <veloxla/core/view.hpp>

#include <veloxla/expression/expr_base.hpp>
#include <veloxla/expression/binary_expr.hpp>
#include <veloxla/expression/unary_expr.hpp>
#include <veloxla/expression/reduction_expr.hpp>
#include <veloxla/expression/eval.hpp>

#include <veloxla/ops/elementwise.hpp>
#include <veloxla/ops/contraction.hpp>
#include <veloxla/ops/matmul.hpp>
#include <veloxla/ops/broadcast.hpp>
