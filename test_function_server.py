# stream_server.py
import socket, time, math, argparse

FUNCS = {
    "x":      lambda x: x,
    "sin":    lambda x: 100*math.sin(x/math.pi)+200,
    "cos":    lambda x: math.cos(x),
    "square": lambda x: x * x,
}
def fun(n, A=1.0, theta_offset=0.0, const_offset=0.0):
    Ts = 0.02              # okres próbkowania (20 ms)
    f = 1.0                # częstotliwość sygnału (Hz)
    t = n * Ts             # czas próbkowania
    return A * math.sin(2 * math.pi * f * t + theta_offset) + const_offset

def main(host="127.0.0.1", port=9000, func_name="x"):
    func = FUNCS.get(func_name, FUNCS["x"])
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((host, port))
        srv.listen(1)
        print(f"Listening on {host}:{port}, streaming f(x) = {func_name}")
        conn, addr = srv.accept()
        with conn:
            print(f"Client connected from {addr}")
            x = 0
            while True:
                value = fun(x, 50, 0, 200);
                conn.sendall(f"I (1234) csv: {value}; {value+100};\n".encode("utf-8"))
                time.sleep(0.1)
                x += 1

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--func", choices=list(FUNCS.keys()), default="x",
                   help="function to stream: x, sin, cos, square")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, default=9000)
    args = p.parse_args()
    main(args.host, args.port, args.func)
