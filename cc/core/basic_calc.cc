#include "core/basic_calc.h"

#include <cmath>
#include <unordered_map>
#include <stack>
#include <string>
#include <utility>
#include <variant>
#include <optional>

namespace {

class Operation final {
 public:
  enum class Type : bool { kUnary, kBinary };

  enum class Priority : short int {
    kBrace,
    kSimple,
    kComplex,
    kFunction,
    kSign
  };

  Operation() = delete;
  ~Operation() = default;

  explicit Operation(Type type, Priority priority,
                     double (*operation)(double, double)) noexcept
      : operation_(operation), priority_(priority), type_(type) {}

  explicit Operation(Type type, Priority priority,
                     double (*operation)(double)) noexcept
      : operation_(operation), priority_(priority), type_(type) {}

  [[nodiscard]] Priority get_priority() const noexcept { return priority_; }
  [[nodiscard]] Type get_type() const noexcept { return type_; }

  [[nodiscard]] double Perform(double num) const {
    return std::get<double (*)(double)>(operation_)(num);
  }

  [[nodiscard]] double Perform(double num1, double num2) const {
    return std::get<double (*)(double, double)>(operation_)(num1, num2);
  }

 private:
  std::variant<double (*)(double, double), double (*)(double)> operation_;
  Priority priority_;
  Type type_;
};

const std::unordered_map<std::string_view, Operation> operations_map = {
    {"--", Operation(Operation::Type::kUnary, Operation::Priority::kSign,
                     [](double num) { return -num; })},
    {"++", Operation(Operation::Type::kUnary, Operation::Priority::kSign,
                     [](double num) { return num; })},
    {"sqrt", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                       std::sqrt)},
    {"sin", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                      std::sin)},
    {"cos", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                      std::cos)},
    {"tan", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                      std::tan)},
    {"asin", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                       std::asin)},
    {"acos", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                       std::acos)},
    {"atan", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                       std::atan)},
    {"ln", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                     std::log)},
    {"log", Operation(Operation::Type::kUnary, Operation::Priority::kFunction,
                      std::log10)},
    {"^", Operation(Operation::Type::kBinary, Operation::Priority::kFunction,
                    std::pow)},
    {"*", Operation(Operation::Type::kBinary, Operation::Priority::kComplex,
                    [](double num1, double num2) { return num1 * num2; })},
    {"/", Operation(Operation::Type::kBinary, Operation::Priority::kComplex,
                    [](double num1, double num2) { return num1 / num2; })},
    {"d", Operation(Operation::Type::kBinary, Operation::Priority::kComplex,
                    std::fmod)},
    {"+", Operation(Operation::Type::kBinary, Operation::Priority::kSimple,
                    [](double num1, double num2) { return num1 + num2; })},
    {"-", Operation(Operation::Type::kBinary, Operation::Priority::kSimple,
                    [](double num1, double num2) { return num1 - num2; })},
    {"(", Operation(Operation::Type::kUnary, Operation::Priority::kBrace,
                    [](double) -> double { return 0; })}};


template <typename Tp>
inline std::optional<Tp> StackPop(std::stack<Tp> &stack) {
  if (stack.empty()) {
    return std::nullopt;
  }
  Tp top_val = stack.top();
  stack.pop();
  return top_val;
}

int ShuntYardAlgo(std::stack<Operation>& operations,
                   std::stack<double>& numbers) {
  std::optional<Operation> operation = StackPop(operations);
  if (!operation.has_value()) {
    return kBasicCalcErrorInvalidSyntax;
  }
  std::optional<double> number1 = StackPop(numbers);
  if (!number1.has_value()) {
    return kBasicCalcErrorInvalidSyntax;
  }
  if (operation->get_priority() == Operation::Priority::kBrace) {
    return kBasicCalcErrorBracesNotMatching;
  }
  double res_number;
  if (operation->get_type() == Operation::Type::kUnary) {
    res_number = operation->Perform(number1.value());
  } else {
    std::optional<double> number2 = StackPop(numbers);
    if (!number1.has_value()) {
      return kBasicCalcErrorInvalidSyntax;
    }
    res_number = operation->Perform(number2.value(), number1.value());
  }
  numbers.push(res_number);
  return kBasicCalcErrorSuccess;
}

inline int ShuntYardBrace(std::stack<Operation>& operations,
                           std::stack<double>& numbers) {
  while (!operations.empty() &&
          operations.top().get_priority() != Operation::Priority::kBrace) {
    if (int err = ShuntYardAlgo(operations, numbers); err != kBasicCalcErrorSuccess) {
      return err;
    }
  }
  if (operations.empty()) {
    return kBasicCalcErrorBracesNotMatching;
  }
  operations.pop();
  return kBasicCalcErrorSuccess;
}

inline int ShuntYardOperation(const Operation& operation,
                               std::stack<Operation>& operations,
                               std::stack<double>& numbers) {
  while (!operations.empty() &&
         operation.get_priority() <= operations.top().get_priority()) {
    if (int err = ShuntYardAlgo(operations, numbers); err != kBasicCalcErrorSuccess) {
      return err;
    }
  }
  operations.push(operation);
  return kBasicCalcErrorSuccess;
}

int ProcessOperation(char op, bool prev_was_num,
                     std::stack<Operation>& operations,
                     std::stack<double>& numbers) {
  auto map_it = operations_map.end();
  if (prev_was_num) {
    map_it = operations_map.find(std::string(1, op));
  } else if (op == '-' || op == '+') {
    map_it = operations_map.find(std::string(2, op));
  }
  if (map_it == operations_map.end()) {
    return kBasicCalcErrorIncorrectOperatorUsage;
  }
  ShuntYardOperation(map_it->second, operations, numbers);
  return kBasicCalcErrorSuccess;
}

std::pair<int, size_t> ProcessFunction(const std::string& expr,
                       std::stack<Operation>& operations) {
  std::unordered_map<std::string_view, Operation>::const_iterator map_it;
  size_t size = 0;
  for (; std::isalpha(expr[size]); ++size)
    ;
  map_it = operations_map.find(expr.substr(0, size));
  if (map_it == operations_map.end()) {
    return {kBasicCalcErrorIncorrectFunctionUsage, 0};
  }
  operations.push(map_it->second);
  return {kBasicCalcErrorSuccess, size - 1};
}

std::pair<int, size_t> ProcessNumber(const std::string& expr, bool prev_was_num,
                     std::stack<double>& numbers) {
  size_t n_size;
  double number = std::stod(&expr[0], &n_size);
  if (prev_was_num || n_size == 0) {
    return {kBasicCalcErrorIncorrectNumberUsage, 0};
  }
  numbers.push(number);
  return {kBasicCalcErrorSuccess, n_size - 1};
}

size_t SkipSpace(const std::string& expr) noexcept {
  size_t i = 0;
  for (; std::isspace(expr[i]); ++i)
    ;
  return i;
}

size_t ClosedBraceNext(const std::string& expr) noexcept {
  size_t i = 0;
  for (; expr[i] && expr[i] != ')'; ++i)
    ;
  for (; expr[i] == ')'; ++i)
    ;
  return i;
}

inline bool PowExprAcceptable(char ch) noexcept {
  return std::isalnum(ch) || std::isspace(ch) || ch == '^' || ch == '(' ||
         ch == '.';
}

void FixPower(std::string& expr, size_t i) {
  i += SkipSpace(expr);
  size_t start = i;
  bool has_pow = false;
  while (PowExprAcceptable(expr[i])) {
    if (expr[i] == '^') {
      has_pow = true;
    } else if (expr[i] == '(') {
      i += ClosedBraceNext(&expr[i]);
      continue;
    }
    ++i;
  }
  if (has_pow) {
    expr.insert(start, "(");
    expr.insert(i + 1, ")");
  }
}

inline bool ReplaceXInString(std::string& expr, const std::string& number) {
  char prev = '\0';
  for (size_t i = 0; i < expr.size(); ++i) {
    if (expr[i] == 'x') {
      if (i > 0 && (std::isdigit(prev) || prev == 'x')) {
        return false;
      }
      expr[i] = ' ';
      expr.insert(i, number);
    }
    if (!std::isspace(expr[i])) {
      prev = expr[i];
    }
  }
  return true;
}

} // namespace

double CALL_CONV BasicCalculateExpr(const char* math_expr, int* err) {
  *err = kBasicCalcErrorSuccess;
  bool prev_was_num = false;
  std::string expr(math_expr);
  std::stack<double> numbers;
  std::stack<Operation> operations;
  for (size_t i = 0; i < expr.size(); ++i) {
    switch (expr[i]) {
      case ' ':
      case '\n':
      case '\t':
        break;
      case 'm':
        if (!(expr[i + 1] == 'o' && expr[i + 2] == 'd')) {
          *err = kBasicCalcErrorIncorrectFunctionUsage;
          return 0;
        }
        i += 2;
        [[fallthrough]];
      case '^':
      case '+':
      case '-':
      case '*':
      case '/':
        if (expr[i] == '^') {
          FixPower(expr, i + 1);
        }
        *err = ProcessOperation(expr[i], prev_was_num, operations, numbers);
        if (*err != kBasicCalcErrorSuccess) {
          return 0;
        }
        prev_was_num = false;
        break;
      case '(':
        operations.push(operations_map.at("("));
        break;
      case ')':
        *err = ShuntYardBrace(operations, numbers);
        if (*err != kBasicCalcErrorSuccess) {
          return 0;
        }
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        auto [stat, incr] = ProcessNumber(&expr[i], prev_was_num, numbers);
        if (stat != kBasicCalcErrorSuccess) {
          *err = stat;
          return 0;
        }
        i += incr;
        prev_was_num = true;
      } break;
      case 'a':
      case 's':
      case 'c':
      case 't':
      case 'l': {
        auto [stat, incr] = ProcessFunction(&expr[i], operations);
        if (stat != kBasicCalcErrorSuccess) {
          *err = stat;
          return 0;
        }
        i += incr;
      } break;
      default:
        *err = kBasicCalcErrorInvalidExpr;
        return 0;
    }
  }
  while (!operations.empty()) {
    if (*err = ShuntYardAlgo(operations, numbers); *err != kBasicCalcErrorSuccess) {
      return 0;
    }
  }
  if (numbers.size() != 1) {
    *err = kBasicCalcErrorInvalidExpr;
    return 0;
  }
  return numbers.top();
}

double CALL_CONV BasicCalculateEquation(const char* math_expr, double x, int* err) {
  std::string expr(math_expr);
  if (!ReplaceXInString(expr, std::to_string(x))) {
    *err = kBasicCalcErrorInvalidXExpr;
    return 0;
  }
  return BasicCalculateExpr(expr.c_str(), err);
}
