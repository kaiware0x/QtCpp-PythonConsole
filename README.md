# PythonInterpreter

Python Bindingを用いてインタープリタのUIを自作する。

FreeCADの`PythonConsole`クラスとその周辺クラスを参考にしている。

## 使用フレームワーク

- Qt
- pybind11

## Memo

- `print`や`len`などのビルトイン関数の一覧は
`dir(__builtins__)`で確認できる。
    - https://note.nkmk.me/python-dir-builtins/

`py::hasattr()`などを呼び出すとき、CPythonソース`Objects/typeobject.c`のAssertに引っかかってデバッグ実行で落ちる.
API呼び出しの前にPyErr_Clear()を呼んでおくと引っかからなくなる.

```cpp
/* Internal API to look for a name through the MRO.
   This returns a borrowed reference, and doesn't set an exception! */
PyObject *
_PyType_Lookup(PyTypeObject *type, PyObject *name)
{
    PyObject *res;
    int error;
    PyInterpreterState *interp = _PyInterpreterState_GET();

    unsigned int h = MCACHE_HASH_METHOD(type, name);
    struct type_cache *cache = get_type_cache();
    struct type_cache_entry *entry = &cache->hashtable[h];
    if (entry->version == type->tp_version_tag &&
        entry->name == name) {
        assert(_PyType_HasFeature(type, Py_TPFLAGS_VALID_VERSION_TAG));
        OBJECT_STAT_INC_COND(type_cache_hits, !is_dunder_name(name));
        OBJECT_STAT_INC_COND(type_cache_dunder_hits, is_dunder_name(name));
        return entry->value;
    }
    OBJECT_STAT_INC_COND(type_cache_misses, !is_dunder_name(name));
    OBJECT_STAT_INC_COND(type_cache_dunder_misses, is_dunder_name(name));

    /* We may end up clearing live exceptions below, so make sure it's ours. */
    assert(!PyErr_Occurred());
    
...
...
...
}
```


### EmbとExtのデバッグ実行に関して

- Python側で予めデバッグ用のEXEとDLLをインストールしておく必要がある。
    - python312_d.dll
    - python3_d.dll
    - python_d.exe
- リリースビルド版
- 自作モジュールの名前には末尾に`_d`をつけないと、PATHを通してもImportする際にモジュールが見つからないとエラーが出る。
    - リリース版自作モジュール名 例：
        - extend01.cp312-win_amd64.pyd
    - デバッグ版自作モジュール名 例：
        - extend01_d.cp312-win_amd64.pyd
        - Importするときは`_d`なしのモジュール名でImportする。


## 進捗

### 2023-11-12

- 入力補完絞り込みができるようになった。
- 自作モジュールをImportできるようになった。
- TODO: より複雑なモジュールを作成する。

### 2023-11-11

- 入力補完が表示できる様になった
- TODO: 入力を進めていったときに補完の絞り込みをする

### 2023-11-07

- SyntaxHighlightを実装した。
- 入力補完実装中。

### 2023-11-04

- QPlainTextEditを継承しUIを作成した。
- Pythonコードを実行できるようになった。
- Pythonの出力をhookしてUIに表示できるようになった。
- ">>> "のようなプロンプトを表示できるようにした。
- 入力コマンドのHistrory機能を実装した。
- TODO: SyntaxHighlightを実装する。
- TODO: 入力補完を実装する。


