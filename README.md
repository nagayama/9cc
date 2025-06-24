# 9cc - A Small C Compiler

9cc is a minimal C compiler that supports basic arithmetic expressions. It demonstrates the fundamental concepts of compiler construction including tokenization, parsing, and code generation.

## Features

- **Arithmetic Operations**: Supports addition (+), subtraction (-), multiplication (*), and division (/)
- **Parentheses**: Handles parentheses for expression grouping and precedence
- **Integer Literals**: Processes integer numbers
- **x86-64 Assembly Output**: Generates Intel syntax assembly code
- **Built-in Testing**: Includes unit tests and integration tests

## Architecture

The compiler follows a traditional three-phase design:

### 1. Tokenizer (Lexical Analysis)
- Converts input string into tokens
- Recognizes operators, numbers, and parentheses
- Skips whitespace characters
- Uses a dynamic vector to store tokens

### 2. Parser (Syntax Analysis)
- Implements recursive descent parsing
- Builds an Abstract Syntax Tree (AST)
- Handles operator precedence correctly:
  - Multiplication and division have higher precedence than addition and subtraction
  - Parentheses override default precedence

### 3. Code Generator
- Traverses the AST to generate x86-64 assembly code
- Uses stack-based evaluation
- Outputs Intel syntax assembly

## Building

```bash
make
```

This will compile the `9cc.c` source file into an executable named `9cc`.

## Usage

### Basic Usage

```bash
./9cc "expression"
```

The compiler takes a mathematical expression as a command-line argument and outputs x86-64 assembly code to stdout.

### Examples

```bash
# Simple addition
./9cc "1+2"

# Complex expression with precedence
./9cc "5+6*7"

# Expression with parentheses
./9cc "5*(9-6)"

# Division
./9cc "(3+5)/2"
```

### Compiling and Running Generated Code

To execute the generated assembly:

```bash
./9cc "1+2*3" > tmp.s
gcc -o tmp tmp.s
./tmp
echo $?  # Shows the result (7)
```

## Testing

### Unit Tests

Run the built-in unit tests:

```bash
./9cc -test
```

### Integration Tests

Run the full test suite:

```bash
make test
```

This will:
1. Run unit tests
2. Execute integration tests that verify various expressions
3. Test cases include:
   - Simple numbers: `0`, `42`
   - Addition and subtraction: `5+20-4`
   - Multiplication precedence: `5+6*7`
   - Parentheses: `5*(9-6)`, `(3+5)/2`
   - Whitespace handling: ` 5 + 20 - 4 `

## Docker Support

A Dockerfile is provided for containerized builds:

```bash
docker build -t 9cc .
docker run -it 9cc
```

## Implementation Details

### Data Structures

- **Token**: Represents lexical tokens with type, value, and position information
- **Node**: AST node structure for representing expressions
- **Vector**: Dynamic array implementation for storing tokens

### Grammar

The compiler recognizes expressions with the following grammar:

```
expr    = mul ("+" mul | "-" mul)*
mul     = term ("*" term | "/" term)*
term    = num | "(" expr ")"
```

### Limitations

- Only supports integer arithmetic
- No variable declarations or assignments
- No control flow statements
- Limited to basic arithmetic operators
- No function definitions

## Files

- `9cc.c` - Main compiler source code
- `Makefile` - Build configuration
- `test.sh` - Integration test script
- `Dockerfile` - Container build configuration
- `.gitignore` - Git ignore patterns

## Educational Purpose

This compiler serves as an excellent introduction to compiler construction concepts. It demonstrates:

- How to implement a tokenizer/lexer
- Recursive descent parsing techniques
- AST construction and traversal
- Basic code generation for x86-64
- Test-driven development in C

The codebase is intentionally simple and well-structured, making it ideal for learning compiler fundamentals.
