#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""KORA EYE FILM SHOW — Raspberry Pi filming director.

Public companion panel for Kora dual-eye firmwares:
Human V18, Metal V18, Monster V18 and Animal V14.

It controls ONLY the eye ESP32-S3 over USB serial. It does not control
Kora's mechanical head, legs or Servo2040. Nothing is transmitted until
the user explicitly chooses a serial port and presses CONNECT.
"""

import glob
import json
import os
import queue
import threading
import time
import tkinter as tk
from tkinter import ttk, messagebox

try:
    import serial
except ImportError:
    serial = None

BAUD = 115200
CONFIG_PORTS = "/home/marcin/vega_robot/config/ports.json"
BLOCKED_PORT_WORDS = ("micropython_board", "servo2040", "pimoroni")

IRIS_BY_THEME = {
    "HUMAN": ["BLUE", "GREEN", "HAZEL"],
    "METAL": ["GREY", "RED", "PURPLE"],
    "MONSTER": ["RED", "PURPLE", "GREEN"],
    "ANIMAL": [],
}
ALL_IRISES = ["ORIGINAL", "BLUE", "GREEN", "GREY", "HAZEL", "RED", "PURPLE"]


def candidate_ports():
    found = []
    try:
        with open(CONFIG_PORTS, "r", encoding="utf-8") as handle:
            cfg = json.load(handle)
        eyes = cfg.get("eyes")
        if isinstance(eyes, str) and eyes and os.path.exists(eyes):
            found.append(eyes)
    except Exception:
        pass

    for pattern in (
        "/dev/serial/by-path/*",
        "/dev/serial/by-id/*",
        "/dev/ttyUSB*",
        "/dev/ttyACM*",
    ):
        for port in sorted(glob.glob(pattern)):
            low = port.lower()
            if any(word in low for word in BLOCKED_PORT_WORDS):
                continue
            if port not in found:
                found.append(port)
    return found


class EyeFilmShow(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("KORA • EYE FILM SHOW")
        self.geometry("1180x820")
        self.minsize(1050, 720)

        self.ser = None
        self.ser_lock = threading.Lock()
        self.reader_stop = threading.Event()
        self.sequence_stop = threading.Event()
        self.sequence_running = False
        self.uiq = queue.Queue()

        self.port_var = tk.StringVar()
        self.conn_var = tk.StringVar(value="DISCONNECTED")
        self.cue_var = tk.StringVar(value="Ready to film / Gotowa do nagrania")
        self.command_var = tk.StringVar()

        self._build_ui()
        self.refresh_ports()
        self.after(80, self._drain_ui_queue)
        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def _build_ui(self):
        tk.Label(self, text="KORA • EYE FILM SHOW", font=("TkDefaultFont", 22, "bold")).pack(pady=(12, 6))
        tk.Label(
            self,
            text="Raspberry Pi filming director • eyes only • no mechanical head / legs / Servo2040",
        ).pack(pady=(0, 10))

        top = ttk.LabelFrame(self, text="1. ESP32-S3 USB connection")
        top.pack(fill="x", padx=12, pady=6)
        self.port_box = ttk.Combobox(top, textvariable=self.port_var, state="readonly", width=76)
        self.port_box.grid(row=0, column=0, padx=8, pady=8, sticky="ew")
        ttk.Button(top, text="REFRESH", command=self.refresh_ports).grid(row=0, column=1, padx=4)
        ttk.Button(top, text="CONNECT", command=self.connect_serial).grid(row=0, column=2, padx=4)
        ttk.Button(top, text="DISCONNECT", command=self.disconnect_serial).grid(row=0, column=3, padx=4)
        tk.Label(top, textvariable=self.conn_var, font=("TkDefaultFont", 11, "bold")).grid(row=0, column=4, padx=10)
        top.columnconfigure(0, weight=1)

        cue = ttk.LabelFrame(self, text="2. Director cue")
        cue.pack(fill="x", padx=12, pady=6)
        tk.Label(cue, textvariable=self.cue_var, font=("TkDefaultFont", 18, "bold"), height=2).pack(fill="x", padx=8, pady=8)

        show = ttk.LabelFrame(self, text="3. Ready-made filming sequences")
        show.pack(fill="x", padx=12, pady=6)
        self.sequence_buttons = []
        themes = ["HUMAN", "METAL", "MONSTER", "ANIMAL"]
        for col, theme in enumerate(themes):
            button = ttk.Button(show, text=f"{theme}\nSHORT", command=lambda t=theme: self.start_sequence(t, "SHORT"))
            button.grid(row=0, column=col, padx=5, pady=5, sticky="ew")
            self.sequence_buttons.append(button)
        for col, theme in enumerate(themes):
            button = ttk.Button(show, text=f"{theme}\nLONG", command=lambda t=theme: self.start_sequence(t, "LONG"))
            button.grid(row=1, column=col, padx=5, pady=5, sticky="ew")
            self.sequence_buttons.append(button)
        light = ttk.Button(show, text="LIGHT DEMO\nGPIO3 / GPIO4", command=self.start_light_demo)
        light.grid(row=2, column=0, columnspan=2, padx=5, pady=6, sticky="ew")
        self.sequence_buttons.append(light)
        tk.Button(show, text="STOP SEQUENCE", command=self.stop_sequence, font=("TkDefaultFont", 12, "bold"), height=2).grid(
            row=2, column=2, columnspan=2, padx=5, pady=6, sticky="ew"
        )
        for col in range(4):
            show.columnconfigure(col, weight=1)

        body = ttk.Frame(self)
        body.pack(fill="both", expand=True, padx=12, pady=6)
        manual = ttk.LabelFrame(body, text="4. Manual controls")
        manual.pack(side="left", fill="y", padx=(0, 6))

        row = ttk.Frame(manual)
        row.pack(fill="x", padx=8, pady=6)
        for label, command in (
            ("WAKE", "WAKE"), ("BLINK", "M"), ("SLEEP", "SLEEP"), ("CENTER", "LOOK 0.00 0.00")
        ):
            ttk.Button(row, text=label, command=lambda c=command: self.send_manual(c)).pack(side="left", padx=3)

        look = ttk.LabelFrame(manual, text="LOOK x y")
        look.pack(padx=8, pady=6)
        points = [
            ("↖", -.70, -.55), ("↑", 0, -.75), ("↗", .70, -.55),
            ("←", -.80, 0), ("●", 0, 0), ("→", .80, 0),
            ("↙", -.70, .55), ("↓", 0, .75), ("↘", .70, .55),
        ]
        for i, (label, x, y) in enumerate(points):
            ttk.Button(look, text=label, width=7, command=lambda xx=x, yy=y: self.send_manual(f"LOOK {xx:.2f} {yy:.2f}")).grid(
                row=i // 3, column=i % 3, padx=3, pady=3
            )

        iris = ttk.LabelFrame(manual, text="IRIS • V18 Human / Metal / Monster")
        iris.pack(fill="x", padx=8, pady=6)
        for i, name in enumerate(ALL_IRISES):
            ttk.Button(iris, text=name, command=lambda n=name: self.send_manual(f"IRIS {n}")).grid(
                row=i // 4, column=i % 4, padx=3, pady=3, sticky="ew"
            )

        custom = ttk.LabelFrame(manual, text="Manual serial command")
        custom.pack(fill="x", padx=8, pady=6)
        entry = ttk.Entry(custom, textvariable=self.command_var, width=38)
        entry.pack(side="left", fill="x", expand=True, padx=4, pady=4)
        entry.bind("<Return>", lambda _event: self.send_custom())
        ttk.Button(custom, text="SEND", command=self.send_custom).pack(side="left", padx=4)

        logs = ttk.LabelFrame(body, text="5. ESP32 / Pi log")
        logs.pack(side="left", fill="both", expand=True, padx=(6, 0))
        self.log_text = tk.Text(logs, wrap="word", state="disabled")
        self.log_text.pack(side="left", fill="both", expand=True, padx=(6, 0), pady=6)
        scroll = ttk.Scrollbar(logs, orient="vertical", command=self.log_text.yview)
        scroll.pack(side="right", fill="y", padx=(0, 6), pady=6)
        self.log_text.configure(yscrollcommand=scroll.set)

        tk.Label(
            self,
            text="LIGHT DEMO uses the real sensors on GPIO3/GPIO4. It does not fake brightness with a LIGHT command.",
            anchor="w",
        ).pack(fill="x", padx=14, pady=(2, 10))

    def log(self, text):
        self.uiq.put(("log", f"[{time.strftime('%H:%M:%S')}] {text}"))

    def cue(self, text):
        self.uiq.put(("cue", text))

    def _drain_ui_queue(self):
        try:
            while True:
                kind, value = self.uiq.get_nowait()
                if kind == "log":
                    self.log_text.configure(state="normal")
                    self.log_text.insert("end", value + "\n")
                    self.log_text.see("end")
                    self.log_text.configure(state="disabled")
                elif kind == "cue":
                    self.cue_var.set(value)
                elif kind == "conn":
                    self.conn_var.set(value)
                elif kind == "buttons":
                    state = "normal" if value else "disabled"
                    for button in self.sequence_buttons:
                        button.configure(state=state)
        except queue.Empty:
            pass
        self.after(80, self._drain_ui_queue)

    def refresh_ports(self):
        ports = candidate_ports()
        current = self.port_var.get()
        self.port_box["values"] = ports
        if current in ports:
            self.port_var.set(current)
        elif ports:
            self.port_var.set(ports[0])
        else:
            self.port_var.set("")
        self.log(f"Found {len(ports)} candidate port(s). Nothing opened.")

    def connect_serial(self):
        if serial is None:
            messagebox.showerror("pyserial missing", "Install it with:\n\nsudo apt install -y python3-serial")
            return
        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning("Port", "Select the ESP32-S3 serial port first.")
            return
        low = port.lower()
        if any(word in low for word in BLOCKED_PORT_WORDS):
            messagebox.showerror("Blocked", "This looks like Servo2040 / MicroPython. The panel will not open it.")
            return
        self.disconnect_serial()
        try:
            opened = serial.Serial(port, BAUD, timeout=.12, write_timeout=.5)
            time.sleep(.25)
            try:
                opened.reset_input_buffer()
            except Exception:
                pass
            with self.ser_lock:
                self.ser = opened
            self.reader_stop.clear()
            threading.Thread(target=self._reader_loop, daemon=True).start()
            self.uiq.put(("conn", f"CONNECTED • {port}"))
            self.log(f"Connected {port} @ {BAUD}. No eye command sent yet.")
        except Exception as exc:
            self.uiq.put(("conn", "DISCONNECTED"))
            messagebox.showerror("Serial", f"Cannot open {port}\n\n{exc}")

    def disconnect_serial(self):
        self.stop_sequence()
        self.reader_stop.set()
        with self.ser_lock:
            opened, self.ser = self.ser, None
        if opened is not None:
            try:
                opened.close()
            except Exception:
                pass
        self.uiq.put(("conn", "DISCONNECTED"))

    def _reader_loop(self):
        while not self.reader_stop.is_set():
            with self.ser_lock:
                opened = self.ser
            if opened is None or not getattr(opened, "is_open", False):
                return
            try:
                raw = opened.readline()
                if raw:
                    line = raw.decode("utf-8", errors="replace").strip()
                    if line:
                        self.log("ESP < " + line)
            except Exception as exc:
                self.log("SERIAL READ ERROR: " + str(exc))
                return

    def _send(self, command):
        command = command.strip()
        if not command:
            return False
        with self.ser_lock:
            opened = self.ser
            if opened is None or not getattr(opened, "is_open", False):
                self.log("NOT SENT — disconnected: " + command)
                return False
            try:
                opened.write((command + "\n").encode("utf-8"))
                opened.flush()
                self.log("PI  > " + command)
                return True
            except Exception as exc:
                self.log("SERIAL WRITE ERROR: " + str(exc))
                return False

    def send_manual(self, command):
        if self.sequence_running:
            self.log("Manual control locked while a sequence is running. Press STOP first.")
            return
        self._send(command)

    def send_custom(self):
        command = self.command_var.get().strip()
        if command:
            self.send_manual(command)
            self.command_var.set("")

    def _connected(self):
        with self.ser_lock:
            return self.ser is not None and getattr(self.ser, "is_open", False)

    def _wait(self, seconds):
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            if self.sequence_stop.is_set():
                return False
            time.sleep(min(.04, max(0, end - time.monotonic())))
        return True

    def _step(self, cue, command=None, pause=.7):
        if self.sequence_stop.is_set():
            return False
        self.cue(cue)
        self.log("CUE • " + cue)
        if command and not self._send(command):
            self.sequence_stop.set()
            return False
        return self._wait(pause)

    def _countdown(self):
        for n in (3, 2, 1):
            self.cue(str(n))
            if not self._wait(.75):
                return False
        self.cue("ACTION")
        return self._wait(.25)

    def _iris_swap(self, name, hold):
        return self._step(f"BLINK → {name}", "M", .10) and self._step(f"IRIS {name}", f"IRIS {name}", hold)

    def _short(self, theme):
        for cue, command, pause in [
            ("WAKE", "WAKE", .8), ("CENTER", "LOOK 0 0", .7), ("BLINK", "M", .55),
            ("LEFT", "LOOK -0.78 0", .9), ("RIGHT", "LOOK 0.78 0", .9),
            ("UP", "LOOK 0 -0.68", .8), ("DOWN", "LOOK 0 0.62", .8),
            ("DIAGONAL ↖", "LOOK -0.62 -0.42", .75), ("DIAGONAL ↘", "LOOK 0.62 0.42", .75),
            ("CENTER", "LOOK 0 0", .6),
        ]:
            if not self._step(cue, command, pause):
                return
        if theme != "ANIMAL":
            for iris in IRIS_BY_THEME[theme]:
                if not self._iris_swap(iris, .75):
                    return
            self._iris_swap("ORIGINAL", .65)
        else:
            for cue, command, pause in [
                ("ANIMAL LEFT", "LOOK -0.45 0.18", .8),
                ("ANIMAL RIGHT", "LOOK 0.45 -0.18", .8),
                ("ANIMAL BLINK", "M", .55),
                ("ANIMAL CENTER", "LOOK 0 0", .65),
            ]:
                if not self._step(cue, command, pause):
                    return
        self._step("SLEEP", "SLEEP", .2)

    def _long(self, theme):
        for cue, command, pause in [
            ("WAKE", "WAKE", 1.1), ("PORTRAIT • CENTER", "LOOK 0 0", 1.4), ("BLINK", "M", .85),
            ("SLOW LEFT", "LOOK -0.72 0", 1.45), ("CENTER", "LOOK 0 0", .7),
            ("SLOW RIGHT", "LOOK 0.72 0", 1.45), ("CENTER", "LOOK 0 0", .7),
            ("UP", "LOOK 0 -0.62", 1.2), ("DOWN", "LOOK 0 0.58", 1.2),
            ("DIAGONAL LEFT", "LOOK -0.58 -0.34", 1.0), ("DIAGONAL RIGHT", "LOOK 0.58 -0.34", 1.0),
            ("CENTER", "LOOK 0 0", 1.0),
        ]:
            if not self._step(cue, command, pause):
                return
        if theme != "ANIMAL":
            for iris in IRIS_BY_THEME[theme]:
                if not self._iris_swap(iris, 1.35):
                    return
                for cue, command in ((f"{iris} • LEFT", "LOOK -0.48 0"), (f"{iris} • RIGHT", "LOOK 0.48 0"), (f"{iris} • CENTER", "LOOK 0 0")):
                    if not self._step(cue, command, .7):
                        return
            self._iris_swap("ORIGINAL", 1.0)
        else:
            for cue, command in [
                ("ANIMAL LEFT", "LOOK -0.62 0.12"), ("ANIMAL RIGHT", "LOOK 0.62 0.12"),
                ("ANIMAL UP", "LOOK 0 -0.48"), ("ANIMAL BLINK", "M"), ("ANIMAL CENTER", "LOOK 0 0"),
            ]:
                if not self._step(cue, command, 1.0):
                    return
        if not self._step("LIGHT SENSOR • CENTER", "LOOK 0 0", .7):
            return
        if not self._step("FLASHLIGHT ON GPIO3/GPIO4 • 4 s", None, 4.0):
            return
        if not self._step("COVER SENSORS / DARK • 4 s", None, 4.0):
            return
        if not self._step("NORMAL LIGHT", None, 2.0):
            return
        self._step("SLEEP", "SLEEP", .2)

    def start_sequence(self, theme, mode):
        if not self._connected():
            messagebox.showwarning("Disconnected", "Select the eye ESP32-S3 port and press CONNECT first.")
            return
        if self.sequence_running:
            return
        self.sequence_stop.clear()
        self.sequence_running = True
        self.uiq.put(("buttons", False))
        threading.Thread(target=self._run_sequence, args=(theme, mode), daemon=True).start()

    def _run_sequence(self, theme, mode):
        try:
            self.cue(f"{theme} • {mode} • camera ready")
            if not self._wait(.7) or not self._countdown():
                return
            (self._short if mode == "SHORT" else self._long)(theme)
            if not self.sequence_stop.is_set():
                self.cue(f"{theme} • {mode} • END")
        finally:
            self.sequence_running = False
            self.uiq.put(("buttons", True))

    def start_light_demo(self):
        if not self._connected():
            messagebox.showwarning("Disconnected", "Select the eye ESP32-S3 port and press CONNECT first.")
            return
        if self.sequence_running:
            return
        self.sequence_stop.clear()
        self.sequence_running = True
        self.uiq.put(("buttons", False))
        threading.Thread(target=self._run_light_demo, daemon=True).start()

    def _run_light_demo(self):
        try:
            if not self._countdown():
                return
            for cue, command, pause in [
                ("WAKE", "WAKE", .8), ("CENTER", "LOOK 0 0", .8),
                ("FLASHLIGHT • gradually brighter • 6 s", None, 6.0),
                ("COVER SENSORS / DARK • 6 s", None, 6.0),
                ("NORMAL LIGHT • 3 s", None, 3.0), ("BLINK", "M", .6), ("SLEEP", "SLEEP", .2),
            ]:
                if not self._step(cue, command, pause):
                    return
            self.cue("LIGHT DEMO • END")
        finally:
            self.sequence_running = False
            self.uiq.put(("buttons", True))

    def stop_sequence(self):
        if self.sequence_running:
            self.sequence_stop.set()
            self.log("STOP SEQUENCE")
            self.cue("STOP")
            self._send("LOOK 0 0")

    def on_close(self):
        self.sequence_stop.set()
        self.reader_stop.set()
        with self.ser_lock:
            opened, self.ser = self.ser, None
        if opened is not None:
            try:
                opened.close()
            except Exception:
                pass
        self.destroy()


if __name__ == "__main__":
    EyeFilmShow().mainloop()
