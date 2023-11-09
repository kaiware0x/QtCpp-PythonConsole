# PythonInterpreter

Python Bindingを用いてインタープリタのUIを自作する。

FreeCADの`PythonConsole`クラスを参考にしている。

## 使用フレームワーク

- Qt
- pybind11

## 進捗

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
