FROM centos:8
LABEL maintainer="developers@moneymanagerex.org"
RUN yum -y --enablerepo=extras install epel-release \
 && yum -y install --setopt=tsflags=nodocs \
      ccache \
      cmake3 \
      gcc-c++ \
      gettext \
      git \
      libcurl-devel \
      make \
      redhat-lsb-core \
      rpm-build \
      wxGTK3-devel \
 && yum clean all

ENV MMEX_INST_CMD yum -y install ./mmex-*.rpm
