import random
import argparse
import sys

def print_mat4x4(m):
    # m is a list of 16 floats in column-major order
    # return a string like: "vec::fmat4{1.000000f, 0.000000f, ...}"
    vals = ", ".join(f"{x:.6f}f" for x in m)
    return f"vec::fmat4{{{vals}}}"

def multiply_mat4x4(a, b):
    # a, b are lists of 16 floats in column-major order
    # result(r,c) = sum_k a(r,k) * b(k,c)
    res = [0.0] * 16
    for c in range(4):
        for r in range(4):
            s = 0.0
            for k in range(4):
                a_rk = a[k*4 + r]   # a(r,k) stored at index k*4 + r (column-major)
                b_kc = b[c*4 + k]   # b(k,c) stored at index c*4 + k
                s += a_rk * b_kc
            res[c*4 + r] = s
    return res

def random_mat4x4():
    # produce 16 floats in a reasonable range
    return [random.uniform(-5.0, 5.0) for _ in range(16)]

def main():
    parser = argparse.ArgumentParser(description="Generate mat4 multiply tests initializer")
    parser.add_argument("count", type=int, nargs="?", default=8, help="number of products to print")
    parser.add_argument("--seed", type=int, help="optional random seed")
    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)

    n = max(0, args.count)

    print("// Generated mat4_multiply_test initializer")
    print("static const mat4_multiply_test tests[] = {")
    for i in range(n):
        a = random_mat4x4()
        b = random_mat4x4()
        e = multiply_mat4x4(a, b)
        print("    { " + f"{print_mat4x4(a)}, {print_mat4x4(b)}, {print_mat4x4(e)} " + "},")
    print("};")

if __name__ == "__main__":
    main()

