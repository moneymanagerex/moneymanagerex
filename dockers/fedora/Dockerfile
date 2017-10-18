FROM fedora:rawhide
MAINTAINER developers@moneymanagerex.org
RUN dnf -y --refresh --nogpg install --setopt=install_weak_deps=False \
        cmake gcc-c++ gettext git make rpm-build redhat-lsb-core \
        webkitgtk4-devel wxGTK3-devel \
        automake libtool ccache \
 && dnf clean all
