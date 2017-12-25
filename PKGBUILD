# Maintainer: kewl fft <kewl@alto.eu.org>

pkgname=dse
pkgver=1.00
pkgrel=1
pkgdesc="AES file encryption using a keyfile."
arch=('any')
url=""
license=('MIT')
source=("http://members.ozemail.com.au/~nulifetv/freezip/freeware/dse-src.zip")
sha256sums=('SKIP')

build() {
  cd "$srcdir/litecoin-$pkgver"
  ./autogen.sh
  ./configure --with-incompatible-bdb --with-gui=qt5
  make
}

package() {
  cd "${srcdir}/$pkgname-$pkgver" || return 1
  install -Dm 0755 sshrc "${pkgdir}/usr/bin/dse"
  #install -Dm 0644 LICENSE "${pkgdir}/usr/share/licenses/$pkgname/LICENSE"
}
