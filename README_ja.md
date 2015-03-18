spihsp
======

spihsp は [Hot Soup Processor](http://hsp.tv/) 2 & 3 向けの Susie プラグインパッケージです。

- **axdpm.spi**: DPM アーカイブ（*.dpm）展開プラグイン、HSP 3.22 以前をサポートします。
- **axdpm33.spi**: DPM アーカイブ（*.dpm）展開プラグイン、HSP 3.3 以降をサポートします。
- **axdpmex.spi**: axdpm.spi と同じですが、EXE 内アーカイブをサポートします。
- **axdpmex33.spi**: axdpm33.spi と同じですが、EXE 内アーカイブをサポートします。

Susie プラグインとは？
------------------------

[Susie](http://www.digitalpad.co.jp/~takechin/) は昔ながらの Windows 向けの画像ビューアです。Susie プラグイン（*.spi）と呼ばれるファイルを追加することで対応フォーマットを増やすことができます。国内ではよくゲーム内部のカスタム画像フォーマットをデコードするのに Susie プラグインが使われます。

Susie プラグインに対応した画像ビューアはいくつか存在します。

- [Susie](http://www.digitalpad.co.jp/~takechin/betasue.html#susie32)
- [Linar](http://hp.vector.co.jp/authors/VA015839/)
- [picture effecter](http://www.asahi-net.or.jp/~DS8H-WTNB/software/index.html)
- [stereophotomaker](http://stereo.jpn.org/eng/stphmkr/)
- [vix](http://www.forest.impress.co.jp/library/software/vix/)
- [A to B converter](http://www.asahi-net.or.jp/~KH4S-SMZ/spi/abc/index.html)
- [ACDSee](http://www.acdsee.com/) (commercial)

わたしのお気に入りは、閲覧目的なら [Linar](http://hp.vector.co.jp/authors/VA015839/)、一括変換目的なら [AtoB Converter](http://www.asahi-net.or.jp/~kh4s-smz/spi/abc/) です。

注意事項・備考など
------------------------

- **axdpm　は `chdpm` で暗号鍵を指定しないアーカイブしかデコードできません**。（これは全自動で暗号鍵を類推するのが困難なことと、axpdm が暗号鍵を指定するためのインタフェースを持たないことに起因します。ちなみに、復号ルーチン自体は完全に通常のデコードと同一です。）
    - そのような場合、暗号鍵を得るのに逆コンパイラを使いたくなるかもしれません。[HSPdeco](http://sourceforge.jp/projects/hspdeco/) などで逆コンパイルして、`chdpm` を探してみてください。
- HSP のファイル復号ルーチンは8ビットの暗号鍵を2つ用いています。したがって、可能なキーの組み合わせは 65,536 パターンしかありません。実際のところ、暗号鍵を取得できなくても総当りでデコードすることが可能です。付属のツール `dpmatt` はそれをシンプルに実装したものです。
- 対象実行ファイルがパッカーで圧縮されている場合（例：[UPX](http://upx.sourceforge.net/)）、axdpm はアーカイブの検出に失敗します。事前に手で実行ファイルを展開してください（詳細は後述のセクションを参照）。
- UNDPM32.DLL は、[統合アーカイバプロジェクト](www.madobe.net/archiver/) API 風の DPM 展開ライブラリです。axdpm.spi は当該ライブラリがなくても動作します。
- 本ツールを用いたことによる、いかなる損害やトラブルについても作者は責任を負いません。

### HSP 製 EXE のアンパック方法

対象実行ファイルがパッカーで圧縮されている場合、手動で展開する必要があります。

1. 対象実行ファイルをバイナリエディタ（例：[QuickBe](http://ninj1nsan.blog.fc2.com/blog-entry-21.html)）で開きます。
2. "DPMX" をファイルの末尾から検索し、"DPMX" とそれ以降の箇所を切り取ります。（取り除いた DPMX セクションは後ほど必要になりますので、どこかに保存しておいてください。)
3. 修正した実行ファイルをアンパッカーで展開します。（例：UPX は UPX によってアンパックすることが可能です）
4. 再度アンパックされた実行ファイルをバイナリエディタで開いて、保存しておいた DPMX セクションをファイルの末尾に復元します。

### HSP 製 EXE のチェックサム

チェックサムの詳細については、ソースコードパッケージの hspsum.cpp をご覧ください。
