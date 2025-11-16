# ![BUDPOP: Belated UDP over Post](./budpop.svg)

## これはなに

[LoLLIPoP](https://github.com/tkytel/lollipop) をつかって、
簡単に気持ちを伝えるためのインタフェイスを提供します。

## ビルド

次の通り実行します。
**budpop** と名付けられた実行ファイルが生成されます。

```console
$ make
```

## 使い方

書式は次の通りです。
**-b** を添えるとブロードキャストできるようになります。

```console
$ budpop [-b] <listen> <destination> <port>
```

- *listen*: リスンするポート番号を指定します。
- *destination*: 宛先の IPv4 アドレスを指定します。
- *port*: 宛先のポート番号を指定します。

## ライセンス

MIT
