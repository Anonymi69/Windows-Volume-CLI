# 🔊 Windows Volume CLI

Simple command-line tool to set your Windows Master volume.

---

## 🚀 Get It

```
git clone https://github.com/Anonymi69/Windows-Volume-CLI.git
cd Windows-Volume-CLI
```

---

## ▶️ Usage

```
volume.exe <0-100>
```

### Example

```
volume.exe 50
```

---

## 🛠️ Build

Use whatever you prefer:

### g++

```
g++ volume.cpp -lole32 -o volume.exe
```

### MSVC (Visual Studio)

```
cl volume.cpp /EHsc /link ole32.lib
```

---

## ⚠️ Notes

* Windows only
* Range is **0–100**
* Controls master system volume

---

## 📄 License

MIT
