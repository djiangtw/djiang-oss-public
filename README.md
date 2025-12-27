# djiang-oss

Personal open source projects and libraries by Danny Jiang.

[![License](https://img.shields.io/badge/License-MIT-blue)](LICENSE)
[![Language](https://img.shields.io/badge/Language-C-lightgrey)]()
[![RISC-V](https://img.shields.io/badge/RISC--V-RV64GC-green)]()
[![Author](https://img.shields.io/badge/Author-Danny%20Jiang-orange)]()

---

## Projects

### [danieRTOS](daniertos/)

A minimal RISC-V RTOS for educational purposes. Built from scratch with 40 accompanying tutorial articles.

**Features:**
- RISC-V 64-bit (RV64GC)
- Cooperative and preemptive scheduling
- Semaphore, Mutex, Queue
- User mode with PMP protection
- SMP multi-core support

**Versions:**
| Version | Codename | Description |
|---------|----------|-------------|
| v0.x | Nano | Basic RTOS: Task, Scheduler, Semaphore, Mutex, Queue |
| v1.x | Secure | User Mode: PMP, Syscall, Fault Handling |
| v2.x | MSMP | M-mode SMP: Spinlock, IPI, Multi-core Scheduler |
| v3.x | SMP | Full Integration: SMP + User Mode + Fault Isolation |

### [djlc](djlc/)

Danny Jiang's LeetCode Library in C. A header-only C library for solving LeetCode problems.

**Features:**
- Hash maps (integer and string keys)
- Heap / Priority Queue
- Stack, Queue, Deque
- Trie, Binary Indexed Tree
- Union-Find, Graph
- And more...

---

## Versioning

This project uses [Semantic Versioning](https://semver.org/) with project-specific prefixes.

### Version Format

```
<project>-v<major>.<minor>.<patch>
```

- **Major**: Architecture or breaking changes
- **Minor**: New features, backward compatible
- **Patch**: Bug fixes, documentation updates

### danieRTOS Versioning

The major version is fixed per architecture variant:

| Directory | Version Range | Example Tags |
|-----------|---------------|--------------|
| `v0-nano/` | v0.x.x | `daniertos-v0.1.0` |
| `v1-secure/` | v1.x.x | `daniertos-v1.0.0` |
| `v2-msmp/` | v2.x.x | `daniertos-v2.0.0` |
| `v3-smp/` | v3.x.x | `daniertos-v3.0.0` |

### djlc Versioning

Standard semantic versioning starting from v1.0.0:

```
djlc-v1.0.0, djlc-v1.1.0, djlc-v1.2.0, ...
```

### Getting a Specific Version

```bash
# List all tags
git tag -l

# Checkout a specific version
git checkout daniertos-v3.0.0
```

---

## 🤝 Contributing

**Feedback Welcome**:

- Open issues for bugs, typos, or suggestions
- Discussion and questions are encouraged

**Note**: This repository aggregates code from private development repositories. Pull requests may not be directly merged but feedback is always appreciated.

---

## 👨‍💻 About the Author

**Danny Jiang**

System software engineer focused on RISC-V architecture, embedded systems, and performance optimization. 20+ years of industry experience, passionate about building educational tools and explaining complex technical concepts.

**Other Works**:

- [Tech Column](https://github.com/djiangtw/tech-column-public) - In-Depth System Architecture and Hardware Design (93 articles)
- [See RISC-V Run: Fundamentals](https://github.com/djiangtw/see-riscv-run-public) - Complete RISC-V Architecture Guide
- [Data Structures in Practice](https://github.com/djiangtw/data-structures-in-practice-public) - Hardware-Oriented Data Structures

---

## 🔗 Links

- **GitHub**: <https://github.com/djiangtw/djiang-oss>
- **Email**: djiang.tw@gmail.com
- **LinkedIn**: [linkedin.com/in/danny-jiang-26359644](https://www.linkedin.com/in/danny-jiang-26359644/)

---

## 📄 License

**Copyright © 2025 Danny Jiang**

This project is licensed under the **MIT License** - see [LICENSE](LICENSE) for details.

---

## 📝 Citation

If you use this project in research, teaching, or articles:

```text
Danny Jiang. (2025). djiang-oss: Open Source Projects and Libraries.
Licensed under MIT. https://github.com/djiangtw/djiang-oss
```

---

**Happy Coding!** 🚀
