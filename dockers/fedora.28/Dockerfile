FROM fedora:28
LABEL maintainer="developers@moneymanagerex.org"
RUN dnf -y install --setopt=install_weak_deps=False \
        cmake gcc-c++ gettext git make rpm-build redhat-lsb-core ccache \
        wxGTK3-devel lua-devel libcurl-devel rapidjson-devel \
 && dnf clean all

ENV MMEX_INST_CMD dnf -y install ./mmex-*.rpm
