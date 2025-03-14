# Yelle's Library of Code

This repository is intended to be a holding room where I can upload all of my general-purpose code, as well as mini-projects that aren't big enough to warrant their own repository.

## Current Contents:

* **`2darray`**: C++ class template for 2D arrays.
  * Plenty of functionality imitates [`<valarray>`](https://en.cppreference.com/w/cpp/numeric/valarray).
  * Much of the functionality will now be achievable by C++23's [`<mdspan>`](https://en.cppreference.com/w/cpp/container/mdspan) and C++26's [`<linalg>`](https://en.cppreference.com/w/cpp/numeric/linalg) as well.
* **`extract-nkdj`**: Python script for searching & extracting data from an [MDict](http://mdict.cn/) version of 日本国語大辞典 *Nihon Kokugo Dai-Jiten* (NKDJ).
  * Depends on the library [`writemdict`](https://github.com/zhansliu/writemdict/tree/master?tab=readme-ov-file) by zhansliu.
  * Requires an MDict `.mdx` file for the NKDJ, not provided.
