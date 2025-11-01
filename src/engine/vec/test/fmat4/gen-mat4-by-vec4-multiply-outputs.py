import random
import argparse
import sys

def print_mat4x4(m):
    # m is a list of 16 floats in column-major order
    vals = ", ".join(f"{x:.6f}f" for x in m)
    return f"vec::fmat4{{{vals}}}"

def print_vec4(v):
    # v is a list of 4 floats [x,y,z,w]
    vals = ", ".join(f"{x:.6f}f" for x in v)
    return f"vec::fvec4{{{vals}}}"

def multiply_mat4_vec4(a, b):
    # a: list[16] column-major, b: list[4]
    # result r_i = sum_k a(i,k) * b_k
    res = [0.0] * 4
    for r in range(4):
        s = 0.0
        for k in range(4):
            a_rk = a[k*4 + r]   # a(row=r, col=k) in column-major
            s += a_rk * b[k]
        res[r] = s
    return res

def random_mat4x4():
    return [random.uniform(-5.0, 5.0) for _ in range(16)]

def random_vec4():
    return [random.uniform(-5.0, 5.0) for _ in range(4)]

def main():
    parser = argparse.ArgumentParser(description="Generate mat4 * vec4 tests initializer")
    parser.add_argument("count", type=int, nargs="?", default=8, help="number of products to print")
    parser.add_argument("--seed", type=int, help="optional random seed")
    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)

    n = max(0, args.count)

    print("// Generated mat4_vec4_multiply_test initializer")
    print("static const mat4_vec4_multiply_test tests[] = {")
    for i in range(n):
        a = random_mat4x4()
        b = random_vec4()
        e = multiply_mat4_vec4(a, b)
        print(f"    {{ {print_mat4x4(a)}, {print_vec4(b)}, {print_vec4(e)} }},")
    print("};")

if __name__ == "__main__":
    main()

