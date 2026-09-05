# Dokumentasi Resmi Bahasa Pemrograman BY# (B-Y-Sharp)

**BY#** adalah bahasa pemrograman berstandar keyboard ASCII dengan sintaks esoterik, ultra-kompak, dan berorientasi efisiensi tinggi. Karakteristik utama BY# adalah filosofi **Binary Text Mandatory**, di mana seluruh teks literal wajib dikodekan dalam bit biner 8-bit `[: ... :]`, serta sistem tipe data berbasis simbol glif 1-karakter (`#`, `~`, `^`, `*`, `%`, `_`).

Interpreter BY# dirancang modular dalam **C++17** dengan arsitektur berlapis: **Lexer**, **AST**, **Parser**, **Environment**, dan **Runtime Evaluator** yang dilengkapi **Standard Library bawaan**, mencakup kalkulasi matematika, manipulasi string, sistem I/O berkas & direktori, hingga kapabilitas jaringan Internet & TCP Socket.

---

## Daftar Isi
1. [Karakteristik & Filosofi Unik](#1-karakteristik--filosofi-unik)
2. [Kamus Lengkap Keywords & Simbol Sintaks](#2-kamus-lengkap-keywords--simbol-sintaks)
3. [Dokumentasi Standard Library](#3-dokumentasi-standard-library)
   - [A. Matematika (Math)](#a-matematika-math)
   - [B. Manipulasi String & Biner](#b-manipulasi-string--biner)
   - [C. Konversi Tipe Data](#c-konversi-tipe-data)
   - [D. Waktu & Sistem](#d-waktu--sistem)
   - [E. I/O Library (Berkas, Direktori & Konsol)](#e-io-library-berkas-direktori--konsol)
   - [F. Internet & Networking Library](#f-internet--networking-library)
4. [Kursus Kilat BY# (Crash Course 10 Modul)](#4-kursus-kilat-by-crash-course-10-modul)
5. [Panduan Instalasi & Build (Termux, Linux & Windows)](#5-panduan-instalasi--build-termux-linux--windows)
   - [A. Instalasi di Android Termux](#a-instalasi-di-android-termux)
   - [B. Instalasi di Linux (Debian/Ubuntu/Arch/Fedora)](#b-instalasi-di-linux-debianubuntuarchfedora)
   - [C. Instalasi di Windows](#c-instalasi-di-windows)
6. [Cara Menjalankan (CLI & REPL)](#6-cara-menjalankan-cli--repl)
7. [Contoh Program Komprehensif](#7-contoh-program-komprehensif)
8. [Lisensi (MIT License)](#8-lisensi-mit-license)

---

## 1. Karakteristik & Filosofi Unik

1. **Binary Text Mandatory (`[: :]`)**:
   - Seluruh teks literal wajib dikemas dalam pembungkus `[:` dan `:]`.
   - Hanya karakter biner `'0'`, `'1'`, dan spasi/newline pemisah byte yang diizinkan.
   - Keberadaan karakter non-biner, kutip biasa (`"..."` atau `'...'`), atau panjang bit bukan kelipatan 8 langsung membatalkan eksekusi dengan pesan:
     ```text
     Syntax Error: Invalid Binary String Literal
     ```
2. **Sistem Glif Tipe Data 1-Karakter**:
   - Menggantikan penulisan tipe verbose dengan glif ASCII ultra-kompak:
     - `#` : Bilangan bulat 64-bit integer (`int`)
     - `~` : Bilangan desimal floating-point (`float`)
     - `^` : Boolean state (`bool`)
     - `*` : Binary string stream (`str`)
     - `%` : Byte data 8-bit unsigned (`byte`)
     - `_` : Void return type (`void`)
3. **Boolean State Ringkas**:
   - `^` : Nilai benar (True / High state)
   - `v` : Nilai salah (False / Low state)
4. **100% Karakter Keyboard ASCII**:
   - Sepenuhnya menggunakan simbol fisik keyboard standar tanpa dependensi font esoterik eksternal.
5. **Cross-Platform & Portabel**:
   - Berjalan mulus di Windows, Linux standar, serta Android Termux tanpa memerlukan pustaka pihak ketiga berukuran besar.

---

## 2. Kamus Lengkap Keywords & Simbol Sintaks

Berikut adalah daftar dan penjelasan mendalam untuk setiap keyword dan simbol operator di BY#:

### A. Keywords & Glif Tipe Data

| Keyword / Glif | Tipe Data Terkait | Peran & Deskripsi | Contoh Kode |
| :---: | :---: | :--- | :--- |
| `#` / `int` | `int` (64-bit signed) | Menyatakan bilangan bulat integer 64-bit bertanda (`int64_t`). Mendukung operasi aritmatika dan bitwise integer. | `@# count <- 10;`<br>`@ int x <- 5;` |
| `~` / `float` | `float` (64-bit double) | Menyatakan bilangan pecahan desimal presisi ganda (`double`). Digunakan untuk perhitungan sains dan matematika akurat. | `@~ rasio <- 3.14159;`<br>`@ float suhu <- 98.6;` |
| `^` / `bool` | `bool` (Boolean) | Menyatakan nilai kebenaran logika. Hanya memiliki dua keadaan: `^` (true) atau `v` (false). | `@^ aktif <- ^;`<br>`@ bool valid <- false;` |
| `*` / `str` | `str` (String) | Tipe teks ASCII/UTF-8 yang didekodekan secara otomatis dari literal biner `[: :]`. | `@* teks <- [: 01001000 01101001 :];` |
| `%` / `byte` | `byte` (8-bit unsigned) | Tipe data angka biner 8-bit tanpa tanda (0 hingga 255). Efisien untuk buffer raw data. | `@% b <- 255;`<br>`@ byte octet <- 65;` |
| `_` / `void` | `void` (Kekosongan) | Menyatakan fungsi yang tidak mengembalikan nilai kembalian apa pun. | `@_ cetak_pesan< > [ > [: 01001111 01001011 :]; ]` |
| `^` / `true` | Nilai Literal Boolean | Menyatakan nilai logika benar (High state / True). | `@^ flag <- ^;` |
| `v` / `false` | Nilai Literal Boolean | Menyatakan nilai logika salah (Low state / False). | `@^ mati <- v;` |
| `break` | Aliran Kontrol Loop | Menghentikan paksa eksekusi perulangan `~` yang sedang aktif dan keluar dari blok perulangan. | `? <i >= 10> [ break; ]` |
| `continue` | Aliran Kontrol Loop | Melewati sisa pernyataan dalam iterasi loop saat ini dan langsung melompat ke iterasi berikutnya. | `? <i % 2 == 0> [ continue; ]` |
| `exit` | Perintah Shell REPL | Perintah terminal interaktif untuk menutup dan keluar dari shell REPL `BY#> `. | `BY#> exit` |

---

### B. Simbol Sintaks & Operator

| Simbol | Nama Simbol | Peran & Penjelasan Sintaks | Contoh Penggunaan |
| :---: | :--- | :--- | :--- |
| `@` | At Sign (Deklarasi) | Memulai deklarasi variabel baru atau deklarasi fungsi baru dalam scope saat ini. | `@# x <- 42;`<br>`@# tambah<# a, # b> [ -> a + b; ]` |
| `<-` | Left Arrow (Assignment) | Operator penugasan nilai dari ekspresi di sisi kanan ke target variabel di sisi kiri. | `x <- x + 1;`<br>`pesan <- [: 01001111 01001011 :];` |
| `->` | Right Arrow (Return) | Operator pengembalian nilai (*return statement*) dari dalam fungsi ke pemanggil fungsi. | `-> a * b;`<br>`-> ;` |
| `?` | Question (Branch/If) | Konstruk percabangan kondisional. Mengevaluasi kondisi di dalam `< >`, jika `true` maka blok dieksekusi. | `? <x > 0> [ > [: 01011001 01100101 01110011 :]; ]` |
| `!` | Exclamation (Else/Not) | Memiliki dua peran: sebagai operator logika NOT jika sebelum ekspresi (`!aktif`), atau cabang ELSE jika setelah blok `?`. | `? <cond> [ ... ] ! [ ... ]`<br>`!aktif` |
| `~` | Tilde (Loop/While) | Konstruk perulangan. Menjalankan blok selama kondisi bernilai benar, atau loop tanpa henti jika tanpa kondisi. | `~ <i < 5> [ i <- i + 1; ]`<br>`~ [ break; ]` |
| `>` | Right Angle (Print Line) | Operator output terminal. Mengevaluasi ekspresi dan mencetak hasilnya ke layar diakhiri baris baru (`\n`). | `> x;`<br>`> <x + y>;` |
| `>>` | Double Right Angle (Print Raw) | Operator output kontinu (*stream emission*). Mencetak hasil ekspresi ke layar secara langsung tanpa baris baru. | `>> [: 01001000 :]; >> [: 01101001 :];` |
| `[: :]` | Binary Text Wrapper | Pembungkus wajib representasi teks biner 8-bit. Seluruh string di BY# wajib berada di dalam simbol ini. | `[: 01001000 01100101 01101100 01101100 01101111 :]` |
| `[ ]` | Square Brackets (Block) | Pembungkus blok kode kumpulan pernyataan (*statement block*) dalam fungsi, kondisi, atau loop. | `[ statement1; statement2; ]` |
| `< >` | Angle Brackets (Envelopes) | Pembungkus argumen kondisi `? < >`, parameter fungsi `f< >`, atau pengelompokan ekspresi matematika `<a + b> * c`. | `? <i < 10> [ ]`<br>`flux<a, b>`<br>`<x * y> + z` |
| `#expr` | Hash Prefix (String Length) | Operator prefiks unari untuk menghitung panjang karakter string atau nilai mutlak angka. | `@# len <- #teks;` |
| `s[i]` | Index Extraction | Operator ekstraksi karakter string berdasarkan indeks berbasis 0, mengembalikan string 1-karakter. | `@* char1 <- teks[0];` |
| `<?` | Input Antenna | Membaca satu baris input teks langsung dari keyboard terminal (`std::cin`). | `@* nama <- <?;` |
| `;` | Semicolon (Terminator) | Penanda batas akhir setiap pernyataan statement dalam bahasa BY#. | `@# x <- 10;` |
| `,` | Comma (Separator) | Pemisah antar parameter fungsi atau argumen pemanggilan fungsi. | `hitung<x, y, z>` |
| `+ - * / %` | Operator Aritmatika | Operasi penambahan, pengurangan, perkalian, pembagian, dan sisa bagi modulo standar. Mendukung string concatenation (`+`). | `x * y + z`, `teks + x` |
| `== != < <= > >=` | Operator Relasional | Membandingkan nilai numerik atau string secara leksikografis, menghasilkan nilai boolean (`^` atau `v`). | `x == 10`, `a <= b` |
| `&& \|\|` | Operator Logika | Evaluasi logika AND (`&&`) dan logika OR (`||`) dengan mekanisme *short-circuit evaluation*. | `x > 0 && y < 10` |

---

## 3. Dokumentasi Standard Library

BY# dilengkapi **Standard Library bawaan** yang siap dipakai langsung tanpa konfigurasi tambahan:

### A. Matematika (`Math`)
* `abs<val>` : Menghasilkan nilai absolut mutlak dari integer atau float.
* `min<a, b>` : Mengembalikan nilai terkecil di antara dua angka.
* `max<a, b>` : Mengembalikan nilai terbesar di antara dua angka.
* `pow<base, exp>` : Menghitung pemangkatan $base^{exp}$ (hasil bertipe float).
* `sqrt<val>` : Menghitung akar kuadrat dari bilangan non-negatif.
* `floor<val>` : Pembulatan angka pecahan ke bawah.
* `ceil<val>` : Pembulatan angka pecahan ke atas.
* `round<val>` : Pembulatan ke integer terdekat.
* `sin<val>`, `cos<val>`, `tan<val>` : Perhitungan fungsi trigonometri radian.
* `log<val>`, `log10<val>` : Logaritma natural ($e$) dan logaritma basis 10.
* `random<min, max>` : Menghasilkan bilangan bulat acak dalam rentang `min` hingga `max`.

### B. Manipulasi String & Biner
* `len<str>` : Menghitung jumlah karakter string (ekuivalen `#str`).
* `substr<str, start, [length]>` : Mengambil potongan substring mulai indeks `start` sepanjang `length`.
* `find<str, needle>` : Mencari posisi indeks pertama kemunculan `needle` dalam `str` (mengembalikan `-1` jika tidak ada).
* `contains<str, needle>` : Mengembalikan boolean `^` (true) jika substring ditemukan, atau `v` (false).
* `replace<str, target, replacement>` : Mengganti seluruh kemunculan kata `target` dengan teks `replacement`.
* `upper<str>` : Mengubah seluruh huruf string menjadi huruf kapital (UPPERCASE).
* `lower<str>` : Mengubah seluruh huruf string menjadi huruf kecil (lowercase).
* `trim<str>` : Menghapus spasi dan baris baru di awal dan akhir string.
* `to_bin<str>` : Mengonversi teks biasa menjadi representasi biner 8-bit terpisah spasi.
* `from_bin<str>` : Mendekodekan representasi string digit biner kembali menjadi teks normal.
* `ord<str>` : Mendapatkan kode nilai ASCII integer dari karakter pertama.
* `chr<int>` : Mengonversi nilai kode integer ASCII menjadi string 1-karakter.

### C. Konversi Tipe Data
* `to_int<val>` : Mengonversi float, byte, boolean, atau string angka ke tipe integer 64-bit (`int`).
* `to_float<val>` : Mengonversi integer, byte, atau string angka ke tipe desimal (`float`).
* `to_str<val>` : Mengonversi tipe data apa pun menjadi representasi string (`str`).
* `to_byte<val>` : Mengonversi angka menjadi nilai integer byte 8-bit (0-255).

### D. Waktu & Sistem
* `time< >` / `clock< >` : Mengembalikan waktu saat ini dalam epoch milidetik (`int`).
* `sleep<ms>` : Menjeda eksekusi program selama `ms` milidetik.
* `sys_exec<cmd>` : Menjalankan perintah shell sistem operasi dan menangkap output string stdout.

### E. I/O Library (Berkas, Direktori, Keyboard & Konsol)
* `file_exists<path>` : Memeriksa keberadaan file di penyimpanan (mengembalikan `^` atau `v`).
* `file_read<path>` : Membaca keseluruhan isi berkas teks menjadi string.
* `file_write<path, content>` : Menuliskan string `content` ke dalam berkas baru (menimpa file lama).
* `file_append<path, content>` : Menambahkan teks string `content` ke akhir file.
* `file_delete<path>` / `file_remove<path>` : Menghapus berkas dari penyimpanan (mengembalikan `^` jika berhasil).
* `file_size<path>` : Mengembalikan ukuran file dalam satuan byte (`int`), atau `-1` jika berkas tidak ditemukan.
* `file_copy<src, dst>` : Menyalin berkas dari path `src` ke path `dst`.
* `file_rename<old_path, new_path>` : Mengubah nama atau memindahkan posisi berkas.
* `file_is_dir<path>` : Memeriksa apakah target adalah sebuah folder / direktori.
* `file_is_file<path>` : Memeriksa apakah target adalah berkas reguler.
* `file_lines_count<path>` : Menghitung jumlah baris dalam berkas teks (`int`).
* `path_join<dir, file>` : Menggabungkan dua komponen path sistem berkas.
* `path_ext<path>` : Mengambil ekstensi berkas (misal: `".by#"`).
* `path_stem<path>` : Mengambil nama berkas tanpa ekstensinya.
* `dir_create<path>` : Membuat folder / direktori baru secara rekursif (mengembalikan boolean `^`).
* `dir_exists<path>` : Memeriksa apakah direktori pada `path` tersedia dan valid.
* `dir_remove<path>` : Menghapus direktori beserta seluruh konten isinya secara rekursif.
* `dir_list<path>` : Mendaftar seluruh nama berkas/folder dalam direktori, dipisahkan oleh karakter baris baru `\n`.
* `console_clear< >` : Membersihkan layar konsol terminal secara instan.
* `console_title<title>` : Mengubah teks judul (*title bar*) jendela konsol terminal.
* `console_cursor<row, col>` : Memindahkan kursor terminal ke posisi baris dan kolom tertentu.
* `console_color<code>` : Menyetel atribut atau warna terminal menggunakan sekuens ANSI.
* `io_read_char< >` / `io_getch< >` : Membaca 1 karakter langsung dari keyboard tanpa perlu menekan Enter.
* `io_kbhit< >` : Memeriksa apakah ada penekanan tombol keyboard aktif (mengembalikan `^` atau `v`).
* `io_flush< >` : Mengosongkan buffer output stdout.
* `io_read_line< >` : Membaca satu baris teks input keyboard (alternatif fungsi dari `<?`).
* `env_get<name>` : Mengambil nilai variabel lingkungan sistem (*environment variable*).
* `env_set<name, val>` : Menyetel nilai variabel lingkungan sistem.


### F. Internet & Networking Library
* `http_get<url>` : Melakukan HTTP / HTTPS GET request ke alamat URL tujuan dan mengembalikan badan respon (*response body*) sebagai string.
* `http_post<url, data>` : Mengirimkan data muatan (*payload*) via HTTP / HTTPS POST request dan mengembalikan hasil respon string.
* `url_encode<str>` : Mengonversi karakter string menjadi format aman URL Encoding (`%XX`).
* `url_decode<str>` : Mengembalikan format URL Encoding (`%XX`) menjadi teks string normal.
* `net_ip_lookup<host>` : Melakukan query DNS untuk mengonversi nama domain menjadi alamat string IPv4 (contoh: `"localhost"` -> `"127.0.0.1"`).
* `net_connect<host, port>` : Membuka koneksi TCP socket ke `host` dan `port`, mengembalikan integer ID socket descriptor (`> 0` jika sukses, `-1` jika gagal).
* `net_send<sock_id, data>` : Mengirim data teks/stream melalui TCP socket terbuka, mengembalikan jumlah byte terkirim.
* `net_recv<sock_id, max_bytes>` : Menerima data stream masuk dari TCP socket hingga batas `max_bytes`, mengembalikan string.
* `net_close<sock_id>` : Menutup koneksi TCP socket dan membebaskan alokasi sistem.
* `net_listen<port>` : Menginisiasi server socket TCP yang mendengarkan (*listen*) koneksi masuk pada `port` tertentu, mengembalikan ID socket server.
* `net_accept<server_sock_id>` : Menerima (*accept*) koneksi klien masuk ke server socket, mengembalikan ID socket klien untuk komunikasi dua arah.

---

## 4. Kursus Kilat BY# (Crash Course 10 Modul)

Pelajari seluruh bahasa BY# dari dasar hingga mahir dalam 10 modul ringkas:

### Modul 1: Aturan Teks Biner & Hello World
Di BY#, string biasa `"Hello"` tidak diizinkan. Setiap huruf dikodekan menjadi 8 bit biner:
- `'H'` = 72 = `01001000`
- `'i'` = 105 = `01101001`
- `'!'` = 33 = `00100001`

```by#
> [: 01001000 01101001 00100001 :];
```

### Modul 2: Sistem Tipe Glif 1-Karakter
Alih-alih menuliskan kata panjang, gunakan glif 1-karakter yang sangat hemat dan padat:
- `@#` = Integer
- `@~` = Float
- `@^` = Boolean
- `@*` = String
- `@%` = Byte

```by#
@# umur <- 25;
@~ berat <- 65.5;
@^ aktif <- ^;
@* salam <- [: 01001000 01101111 01101100 01100001 :];
@% tag <- 128;
```

### Modul 3: Penugasan Nilai (`<-`) & Aritmatika
Operator penugasan menggunakan panah kiri `<-`, bukan tanda sama dengan `=`.
Pengelompokan prioritas dapat menggunakan `< >` atau `( )`:

```by#
@# a <- 10;
@# b <- 20;
@# total <- <a + b> * 2;
> total;
```

### Modul 4: Penggabungan String & Teks Dinamis
Operator `+` otomatis menggabungkan teks dengan angka:

```by#
@# skor <- 100;
> <[: 01010011 01101011 01101111 01110010 00111010 00100000 :] + skor>;
```

### Modul 5: Percabangan Kondisional (`?` dan `!`)
Percabangan menggunakan simbol tanya `? <kondisi> [ blok ]` dan `! [ blok_else ]`:

```by#
@# nilai <- 85;
? <nilai >= 75> [
    > [: 01001100 01010101 01001100 01010101 01010011 :];
] ! [
    > [: 01000111 01000001 01000111 01000001 01001100 :];
]
```

### Modul 6: Perulangan Siklus (`~`)
Perulangan menggunakan tilde `~`. Mendukung perulangan bersyarat maupun siklus bebas:

```by#
@# i <- 1;
~ <i <= 3> [
    > i;
    i <- i + 1;
]

@# loop <- 0;
~ [
    ? <loop >= 2> [ break; ]
    loop <- loop + 1;
]
```

### Modul 7: Membuat Fungsi Kustom
Deklarasi fungsi menggunakan `@` diikuti tipe kembalian, nama fungsi, parameter dalam `< >`, dan blok kode `[ ]`:

```by#
@# kuadrat<# x> [
    -> x * x;
]

@# hasil <- kuadrat<7>;
> hasil;
```

### Modul 8: Ekstraksi Panjang (`#s`) & Indeks (`s[i]`)
Menghitung panjang string cukup meletakkan operator `#` di depan variabel teks. Mengambil karakter tertentu menggunakan bracket `[i]`:

```by#
@* teks <- [: 01001010 01000001 01010110 01000001 :];
> #teks;
> teks[0];
```

### Modul 9: Emisi Kontinu (`>>`) & Input Pengguna (`<?`)
Gunakan `>>` untuk mencetak tanpa baris baru, dan `<?` untuk membaca input terminal:

```by#
>> [: 01001110 01101111 00111010 00100000 :];
> 42;
```

### Modul 10: Penggunaan I/O & Jaringan Internet
Akses berkas, direktori, dan jaringan Internet langsung lewat fungsi bawaan:

```by#
@* path <- [: 01100001 01110000 01101001 00101110 01110100 01111000 01110100 :];
file_write<path, [: 01001111 01001011 :]>;
@* host <- [: 01101100 01101111 01100011 01100001 01101100 01101000 01101111 01110011 01110100 :];
@* ip <- net_ip_lookup<host>;
> ip;
```

---

## 5. Panduan Instalasi & Build (Termux, Linux & Windows)

### A. Instalasi di Android Termux

Di Termux Android, pengguna **TIDAK PERLU mengunduh bahan/compiler berukuran besar** (`clang`/`make`). Pre-compiled binary resmi telah disediakan untuk arsitektur AArch64 (ARM64), ARMv7a (32-bit), dan x86_64:

#### Metode 1: Instalasi Instan 1-Detik (Rekomendasi - Tanpa Download Bahan)
Cukup jalankan satu baris perintah berikut di terminal Termux Anda:
```bash
curl -sL https://raw.githubusercontent.com/sunandar3221/BYSharp/main/install.sh | bash
```
*Skrip otomatis mendeteksi arsitektur CPU ponsel Anda, mengunduh binary siap pakai (~200 KB), dan memasangnya langsung ke `$PREFIX/bin/bys` dan `$PREFIX/bin/by#`.*

#### Metode 2: Dari Git Repository
```bash
git clone https://github.com/sunandar3221/BYSharp.git
cd BYSharp
chmod +x install.sh
./install.sh
```

#### Metode 3: Kompilasi Manual dari Source Code (Opsional)
Jika Anda ingin mengompilasi sendiri dari source code:
```bash
pkg install -y clang make
make
make install
```

Setelah terinstal, perintah `bys` dan `by#` akan langsung aktif di terminal Termux Anda:
```bash
bys --version
```

---

### B. Instalasi di Linux (Debian/Ubuntu/Arch/Fedora)

BY# dapat dikompilasi pada distribusi Linux apa pun yang memiliki compiler C++17 (`g++` atau `clang++`).

#### Debian / Ubuntu / Linux Mint / Kali:
```bash
sudo apt update
sudo apt install -y build-essential git
git clone https://github.com/sunandar3221/BYSharp.git
cd BYSharp
chmod +x install.sh
sudo ./install.sh
```

#### Arch Linux / Manjaro:
```bash
sudo pacman -Syu --noconfirm base-devel git
git clone https://github.com/sunandar3221/BYSharp.git
cd BYSharp
make && sudo make install
```

#### Fedora / RHEL:
```bash
sudo dnf install -y gcc-c++ make git
git clone https://github.com/sunandar3221/BYSharp.git
cd BYSharp
make && sudo make install
```

---

### C. Instalasi di Windows

Kompilasi pada Windows menggunakan MinGW / LLVM-MinGW dengan perintah berikut:

```powershell
g++ -std=c++17 -O2 -I src src/main.cpp src/Lexer.cpp src/Parser.cpp src/Evaluator.cpp -lws2_32 -o bys.exe
copy bys.exe by#.exe
```

---

## 6. Cara Menjalankan (CLI & REPL)

### A. Menjalankan Berkas Program (.by# atau .bys)

```bash
# Menjalankan script uji inti
bys tests/test_core.by#

# Menjalankan script uji standard library
bys tests/test_stdlib.by#

# Menjalankan script uji I/O dan Jaringan
bys tests/test_net_io.by#
```

### B. Shell Interaktif (Alien REPL Console)

Jalankan perintah `bys` atau `by#` tanpa argumen untuk masuk ke prompt interaktif:

```text
$ bys
========================================
  BY# Interactive Shell (v1.0.0)
  Standard: ASCII Keyboard & Binary Text
  Type 'exit' to quit.
========================================
BY#> @# a <- 15;
BY#> @# b <- 25;
BY#> > a + b;
40
BY#> @* s <- [: 01001011 01001111 01000100 01000101 :];
BY#> > s;
KODE
BY#> exit
```

---

## 7. Contoh Program Komprehensif

Program pengujian sistem I/O dan Jaringan [tests/test_net_io.by#](file:///C:/Users/Administrator/Downloads/BY#/tests/test_net_io.by#):

```by#
> [: 01010101 01001010 01001001 00100000 01001001 01001111 00100000 00100110 00100000 01001001 01001110 01010100 01000101 01010010 01001110 01000101 01010100 :];

@* dname <- [: 01110100 01100101 01110011 01110100 01011111 01100100 01101001 01110010 :];
dir_create<dname>;
@^ dexists <- dir_exists<dname>;
> <[: 01000100 01001001 01010010 00100000 01000011 01010010 01000101 01000001 01010100 01000101 01000100 00111010 00100000 :] + dexists>;

@* fname <- [: 01110100 01100101 01110011 01110100 01011111 01101001 01101111 00101110 01110100 01111000 01110100 :];
file_write<fname, [: 01101000 01100101 01101100 01101100 01101111 :]>;
@# fsz <- file_size<fname>;
> <[: 01000110 01001001 01001100 01000101 00100000 01010011 01001001 01011010 01000101 00111010 00100000 :] + fsz>;

@* fcopy <- [: 01110100 01100101 01110011 01110100 01011111 01100011 01101111 01110000 01111001 00101110 01110100 01111000 01110100 :];
file_copy<fname, fcopy>;
@* frename <- [: 01110100 01100101 01110011 01110100 01011111 01100010 01111001 01110011 00101110 01110100 01111000 01110100 :];
file_rename<fcopy, frename>;
file_delete<frename>;
file_delete<fname>;
dir_remove<dname>;

@* raw_s <- [: 01101000 01100101 01101100 01101100 01101111 00100000 01110111 01101111 01110010 01101100 01100100 00100000 00100110 00100000 01100010 01111001 00100011 :];
@* enc <- url_encode<raw_s>;
> <[: 01000101 01001110 01000011 01001111 01000100 01000101 01000100 00111010 00100000 :] + enc>;
@* dec <- url_decode<enc>;
> <[: 01000100 01000101 01000011 01001111 01000100 01000101 01000100 00111010 00100000 :] + dec>;

@* host <- [: 01101100 01101111 01100011 01100001 01101100 01101000 01101111 01110011 01110100 :];
@* ip <- net_ip_lookup<host>;
> <[: 01001001 01010000 00100000 01001100 01001111 01000011 01000001 01001100 01001000 01001111 01010011 01010100 00111010 00100000 :] + ip>;

@* cmd_str <- [: 01100101 01100011 01101000 01101111 00100000 01000010 01011001 00100011 01011111 01001111 01001011 :];
@* out <- trim<sys_exec<cmd_str>>;
> <[: 01010011 01011001 01010011 00100000 01000101 01011000 01000101 01000011 00111010 00100000 :] + out>;

> [: 01001001 01001111 00100000 00100110 00100000 01001001 01001110 01010100 01000101 01010010 01001110 01000101 01010100 00100000 01010011 01000101 01001100 01000101 01010011 01000001 01001001 :];
```

**Hasil Eksekusi Terminal:**
```text
UJI IO & INTERNET
DIR CREATED: true
FILE SIZE: 5
ENCODED: hello%20world%20%26%20by%23
DECODED: hello world & by#
IP LOCALHOST: 127.0.0.1
SYS EXEC: BY#_OK
PATH JOIN: my_folder\file.by#
PATH EXT: .by#
PATH STEM: file
ENV VAL: BY_VALUE_OK
IO & INTERNET SELESAI
```

---

## 8. Lisensi (MIT License)

Proyek bahasa pemrograman **BY#** didistribusikan di bawah lisensi terbuka **[MIT License](LICENSE)**. Anda bebas menggunakan, memodifikasi, mendistribusikan, menyatukan, dan mengomersialkan perangkat lunak ini secara bebas.

