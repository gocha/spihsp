spihsp
======

spihsp is a Susie plugin package for [Hot Soup Processor](http://hsp.tv/) 2 and 3.

- **axdpm.spi**: DPM archive (*.dpm) unpacker, supports HSP 3.22 or older.
- **axdpm33.spi**: DPM archive (*.dpm) unpacker, supports HSP 3.3 or later.
- **axdpmex.spi**: Same as axdpm.spi, but this one supports in-EXE archive.
- **axdpmex33.spi**: Same as axdpm33.spi, but this one supports in-EXE archive.

What is Susie plugin?
------------------------

[Susie](http://www.digitalpad.co.jp/~takechin/) is an old-school picture viewer for Windows, which can support additional formats by adding Susie plugins (*.spi). In Japan, Susie plugin is often used for decoding custom image formats inside a game.

Susie is not a only software which supports Susie plugin. My favorite tools are [Linar](http://hp.vector.co.jp/authors/VA015839/) for browsing, and [AtoB Converter](http://www.asahi-net.or.jp/~kh4s-smz/spi/abc/) for batch conversion.

Note
------------------------

- **axdpm can decrypt only archives that do not specify the encryption key by `chdpm`**. (because it is hard to determine the encryption key 100% automatically, and axdpm does not have an interface for specifying encryption key. FYI, decryption routine is exactly identical to the regular decoding.)
    - In such case, you may want to use a HSP decompiler to get the encryption key. Try [HSPdeco](http://sourceforge.jp/projects/hspdeco/) and search `chdpm` from the decompiled source code.
- HSP file decryption routine uses two 8 bit encryption keys. Therefore, possible key combinations are only 65,536 patterns. In fact, you can try brute force decoding even if you cannot get the encryption key. A bundled tool `dpmatt` is a simple implementation of that.
- If target EXE is compressed by a packer (e.g. [UPX](http://upx.sourceforge.net/)), axdpm will fail at archive detection. Please unpack the EXE by hand beforehand (see the following section for details).
- UNDPM32.DLL is a DPM unpacker library that respects [Common Archiver Project](www.madobe.net/archiver/) API. axdpm.spi can work without it.
- By the way, do you know that HSP has a simple checksum mechanism? I explained its algorithm at this page <http://d.hatena.ne.jp/GOCHA/20060212/hsp3checksum>
- Any use of the software is entirely at your own risk.

### How to unpack HSP-EXE

If target EXE is compressed by a packer, you need to unpack the EXE by hand.

1. Open the target EXE in a hex editor (e.g. [Hex Workshop](http://www.hexworkshop.com/)).
2. Search for "DPMX" from the tail of file. Then, cut the "DPMX" itself, and the part after it. (Note that the removed DPMX section is needed a little later. So please save it to somewhere.)
3. Unpack the modified EXE by using unpacker tool. (e.g. UPX can be unpacked by UPX)
4. Reopen the unpacked EXE in a hex editor, and recover the preserved DPMX section into the tail of file.
