# CPU
* CPUはメモリに読み書きしながら処理を行う
* CPUが実行するプログラムと読み書きするデータは両方メモリにある。
* 現在実行中の命令のアドレスをCPU内部に保持している。
    * そのアドレスから命令を読み出してそこに書かれている事を行い。次の命令を読みだして実行する。
    * 現在実行中の命令のアドレスをプログラムカウンタと言う。
* CPUが実行するプログラムの形式は機械語
* プログラムカウンタ以外にも少数のデータ保存領域がある。これをレジスタと言う。
    * x86-64だと64bitの整数を16個保持出来る。
    * レジスタはCPU内部に存在するため遅延なしでアクセス出来る。
* 基本のプログラム実行の流れはCPUがメモリからレジスタにデータを読み込んで、レジスタとレジスタで何らかの演算を行い、その結果をメモリに書き戻すという形になる。
* 特定の機械語の命令を総称として **命令セットアーキテクチャ**、あるいは単に命令セットと言う。
    * PCではIntelとその互換チップのAMDの **x86-64**が使われている。
    * 組み込みだと **ARM**が主流
