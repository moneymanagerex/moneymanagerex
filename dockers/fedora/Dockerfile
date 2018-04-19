FROM fedora:rawhide
MAINTAINER developers@moneymanagerex.org
RUN dnf -y --refresh --nogpg install --setopt=install_weak_deps=False \
        cmake gcc-c++ gettext git make rpm-build redhat-lsb-core ccache \
        webkitgtk4-devel wxGTK3-devel libcurl-devel \
 && dnf clean all

ENV MMEX_INST_CMD dnf -y install ./mmex-*.rpm
