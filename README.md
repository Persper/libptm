# libptm

[![Run Status](https://api.shippable.com/projects/57792fd03be4f4faa56ba5e1/badge?branch=master)]()

A minimal library of persistent transactional memory (PTM).

**NOTE**: This repo is under construction. For now, it is a reference interface
design for PTM (see below), but not a fully working system. Please let us know
if you like it -- your demand will drive our development.

# LibPTM’s *One-Page* Manual

LibPTM’s way to manage and access persistent memory data aims for *simplicity*.
If you can't be bothered to read tens of pages of [NVML's manual](http://pmem.io/nvml/), learn LibPTM in ten minutes.

Persistent memory data lives longer than processes. It is managed by the
operating system (OS) as files. LibPTM associates each process
exclusively with a single data file. So, simply call `popen(“data/file/path”)`,
perhaps in the beginning of your program, to get started.

## Allocation

To allocate and give back volatile memory space, we call `malloc()`/`free()` in
C or `new`/`delete` in C++. LibPTM provides their counterparts for persistent
memory.

```c++
int *n = (int *)pmalloc(sizeof(int));
pfree(n);
int *n = pnew(int, 2);
pdelete(n);
```

## Naming

LibPTM provides a way to assign a unique string ID to a persistent memory data
structure, so that the program can retrieve it even after the process shuts
down.  Suppose we store a tree structure on persistent memory, then the C++
code can go like this.

```c++
Tree *tree = (Tree *)pretrieve(“unique-id”);
if (!tree) { // In case it is not created yet:
  tree = pnew(Tree); // Create the tree.
  pregister(“unique-id”, tree); // Give it a name.
} // Hereafter use the tree as usual.
```

Usually only a very small number of such “seed” IDs are necessary, as most of
data is referenced and reachable from a few seeds. If a seed ID is not needed
any longer, call `pderegister(“unique-id”)`.

## Annotation

All accesses to persistent memory data have to be wrapped in special
annotations in order to protect data consistency against a crash. Wrap code
blocks in `__persistent { }`. In case of a crash, persistent memory rolls back
data changes in any interrupted such block. Note that a libptm transaction only
guarantees durability and atomicity. Use other mechanisms such as locking to
realize concurrency control. The following code snippet shows the way to
annotate code blocks.
```c++
tree_lock.lock();
__ptm {
  tree->insert(value);
  ++num_values;
}
tree_lock.unlock();
```
That’s it. Enjoy your persistent memory!

## Design Rationale

LibPTM achieves simplicity for the following reasons.

* We assume each process has only one persistent data file. To our
understanding, persistent memory data plays a role similar to the heap of a
process rather than a traditional file. The purpose of organizing persistent
data into files is to facilitate management in OS, but we don’t regard them as
traditional files when using them in processes. Therefore, we simplify the
design by only allowing one data file per process.

* A single persistent memory data file is mapped to a process at a fixed
virtual memory address.  That means addresses of persistent memory data remain
the same even if the process restarts.  Therefore, it is safe to reference
persistent data using virtual memory addresses and regular pointers. In
contrast, NVML uses a location-independent wide address and special pointers
for persistent memory data.  That causes both performance penalty and
programming burden.

* We utilize instrumentation to avoid annotating every persistent memory write.
This is realized by [libitm](https://gcc.gnu.org/onlinedocs/libitm/index.html),
in our prototype. The PTM runtime automatically checks whether a write address
falls in persistent memory or not. DRAM writes are filtered out without
programmers' manual work.

The three design choices are mutually linked. A single persistent data file per
process enables the fixed location of persistent memory in the virtual address
space. Only when the persistent memory region is fixed, runtime filtering can
be done efficiently, with one or two extra CPU cycles per memory write;
otherwise, it may cost a table lookup to determine whether a memory address is
in the persistent memory region(s) or not, hence the filtering overhead would
be prohibitively high.

## Academic Citation

BibTex:
```
@inproceedings{Ren:2017:PNM:3124680.3124729,
 author = {Ren, Jinglei and Hu, Qingda and Khan, Samira and Moscibroda, Thomas},
 title = {Programming for Non-Volatile Main Memory Is Hard},
 booktitle = {Proceedings of the 8th Asia-Pacific Workshop on Systems},
 series = {APSys~'17},
 year = {2017},
 isbn = {978-1-4503-5197-3},
 location = {Mumbai, India},
 pages = {13:1--13:8},
 articleno = {13},
 numpages = {8},
 url = {http://doi.acm.org/10.1145/3124680.3124729},
 doi = {10.1145/3124680.3124729},
 acmid = {3124729},
 publisher = {ACM},
}
```
