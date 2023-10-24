project('haclog', 'c', license: 'MIT')

################################
# version
################################
fs = import('fs')

haclog_version = fs.read('version.txt').strip()
haclog_version_without_ext = haclog_version.split('-')[0]
haclog_semver = haclog_version_without_ext.split('.')
haclog_version_major = haclog_semver[0]
haclog_version_minor = haclog_semver[1]
haclog_version_patch = haclog_semver[2]
haclog_soversion = haclog_version_major

message('--------------------------------')
message('# haclog version:', haclog_version)
message('# haclog soversion:', haclog_soversion)
message('--------------------------------')

################################
# configure
################################

conf_data = configuration_data()
conf_data.set('HACLOG_VERSION', haclog_version)
configure_file(
  format: 'cmake@',
  input: 'haclog/haclog_config.h.in',
  output: 'haclog_config.h',
  configuration: conf_data)

# NOTE:
# configure_file output can't contain a path segment, so need move gen 
# file manually
run_command('python', '-c',
  '''
import os;
build_dir = "@0@";
src_dir = "@1@";
gen_dir = os.path.join(src_dir, "gen", "haclog");
os.makedirs(gen_dir, exist_ok=True);
src_file = os.path.join(build_dir, "haclog_config.h");
dst_file = os.path.join(gen_dir, "haclog_config.h");
os.rename(src_file, dst_file);
  '''.format(meson.current_build_dir(), meson.current_source_dir()),
  check: true)

################################
# haclog
################################

src_fmt ='''import glob;
files = glob.glob("haclog/**/*.@0@", recursive=@1@);
for f in files:
  print("{}".format(f))
'''

src_c = run_command('python', '-c', src_fmt.format('c', 'True'), check: true)
haclog_src_c = src_c.stdout().strip().split('\n')

incdir = include_directories('.', 'gen')
library('haclog', haclog_src_c,
  include_directories: incdir,
  # NOTE: meson not support semver ext, e.g. 0.0.1-alpha.1
  #version: haclog_version,
  version: haclog_version_without_ext,
  soversion: haclog_soversion,
  install: true)

# TODO: install headers
#src_h = run_command('python', '-c', src_fmt.format('h', 'True'), check: true)
#haclog_src_h = src_h.stdout().strip().split('\n')
