# PythonInterpreter

Python Bindingを用いてインタープリタのUIを自作する。

FreeCADの`PythonConsole`クラスを参考にしている。

## 使用フレームワーク

- Qt
- pybind11

## Memo

- `print`や`len`などのビルトイン関数の一覧は
`dir(__builtins__)`で確認できる。
    - https://note.nkmk.me/python-dir-builtins/

CPythonソース末尾のAssertに引っかかってデバッグ実行で落ちる.
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


## 進捗

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


