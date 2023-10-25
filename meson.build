project('haclog', 'c',
  version: run_command('python', '-c', '''
with open("version.txt", "r") as f:
    print("{}".format(f.readline().strip("\n")))
    ''', check: true).stdout().strip(),
  license: 'MIT',
  license_files: ['LICENSE'],
  meson_version: '>= 1.1.0')

################################
# version
################################

haclog_version = meson.project_version()
haclog_version_without_ext = haclog_version.split('-')[0]
haclog_semver = haclog_version_without_ext.split('.')
haclog_version_major = haclog_semver[0]
haclog_version_minor = haclog_semver[1]
haclog_version_patch = haclog_semver[2]
haclog_soversion = haclog_version_major

message('--------------------------------')
message('# haclog version:', haclog_version)
message('# haclog soversion:', haclog_soversion)
message('# default_library:', get_option('default_library'))
message('# haclog HACLOG_BUILD_TESTING:', get_option('HACLOG_BUILD_TESTING'))
message('# haclog HACLOG_BUILD_EXAMPLE:', get_option('HACLOG_BUILD_EXAMPLE'))
message('# haclog HACLOG_BUILD_BENCHMARK:',
  get_option('HACLOG_BUILD_BENCHMARK'))
message('# haclog HACLOG_BUILD_GBENCHMARK:',
  get_option('HACLOG_BUILD_GBENCHMARK'))
message('--------------------------------')

################################
# configure
################################

conf_data = configuration_data()
conf_data.set('HACLOG_VERSION', haclog_version)

if get_option('default_library') == 'shared'
  conf_data.set('HACLOG_USE_DLL', 1)
elif get_option('default_library') == 'static'
else
  assert(false, '''
haclog not support build shared and static in the same time
setup --default-library=shared or --default-library=static
  ''')
endif

if build_machine.system() != 'windows'
  compiler = meson.get_compiler('c')
  if compiler.has_header('execinfo.h')
    if compiler.has_function('backtrace', prefix: '#include <execinfo.h>')
      conf_data.set('HACLOG_HAVE_BACKTRACE', 1)
      conf_data.set('HACLOG_BACKTRACE_HEADER', 'execinfo.h')
    endif
  endif
endif

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
gen_dir = os.path.join(build_dir, "gen", "haclog");
os.makedirs(gen_dir, exist_ok=True);
src_file = os.path.join(build_dir, "haclog_config.h");
dst_file = os.path.join(gen_dir, "haclog_config.h");
os.replace(src_file, dst_file);
  '''.format(meson.current_build_dir().replace('\\', '/')),
  check: true)

################################
# functions
################################

func_glob ='''import glob;
files = glob.glob("@0@/**/*.@1@", recursive=@2@);
for f in files:
    print("{}".format(f))
'''

func_listdir = '''import os;
dirs = os.listdir("@0@");
for filename in dirs:
    filepath = os.path.join("@0@", filename)
    if os.path.isdir(filepath):
        print(filename)
'''

################################
# haclog
################################

cmd_src_c = run_command(
  'python', '-c', func_glob.format('haclog', 'c', 'True'), check: true)
cmd_src_h = run_command(
  'python', '-c', func_glob.format('haclog', 'h', 'True'), check: true)
haclog_src_c = cmd_src_c.stdout().strip().split('\n')
haclog_src_h = cmd_src_h.stdout().strip().split('\n')
haclog_gen_h = join_paths(
  meson.current_build_dir(), 'gen', 'haclog', 'haclog_config.h')

incdir = include_directories('.', 'gen')
libhaclog = library('haclog', haclog_src_c,
  include_directories: incdir,
  c_args: '-DHACLOG_EXPORTS',
  # NOTE: meson not support semver ext, e.g. 0.0.1-alpha.1
  version: haclog_version_without_ext,
  soversion: haclog_soversion,
  install: true)
install_headers(haclog_gen_h, subdir: 'haclog')
install_headers(haclog_src_h, preserve_path: true)

# pkg-config
pkg = import('pkgconfig')
pkg.generate(libhaclog)

################################
# test
################################

if get_option('HACLOG_BUILD_TESTING')
  unity_test_dep = dependency('unity', required: true)

  cmd_listdir_test = run_command(
    'python', '-c', func_listdir.format('test'), check: true)
  test_dirs = cmd_listdir_test.stdout().strip().split('\n')

  foreach test_dir : test_dirs
    test_filepath = join_paths('test', test_dir).replace('\\', '/')
    cmd_test_src_c = run_command(
      'python', '-c', func_glob.format(test_filepath, 'c', 'True'),
      check: true)
    test_src_c = cmd_test_src_c.stdout().strip().split('\n')
    test_incdir = include_directories(test_filepath, '.', 'gen')
    test_exe = executable('test_' + test_dir, test_src_c,
      include_directories: test_incdir,
      link_with: libhaclog,
      dependencies: unity_test_dep)
    test('name of test', test_exe)
  endforeach
endif

################################
# examples
################################

if get_option('HACLOG_BUILD_EXAMPLE')
  cmd_listdir_example = run_command(
    'python', '-c', func_listdir.format('example'), check: true)
  example_dirs = cmd_listdir_example.stdout().strip().split('\n')

  foreach example_dir : example_dirs
    example_filepath = join_paths('example', example_dir).replace('\\', '/')
    cmd_example_src_c = run_command(
      'python', '-c', func_glob.format(example_filepath, 'c', 'True'),
      check: true)
    example_src_c = cmd_example_src_c.stdout().strip().split('\n')
    example_incdir = include_directories(example_filepath, '.', 'gen')
    executable('example_' + example_dir, example_src_c,
      include_directories: example_incdir,
      link_with: libhaclog)
  endforeach
endif

################################
# benchmark
################################

if get_option('HACLOG_BUILD_BENCHMARK')
  cmd_listdir_benchmark = run_command(
    'python', '-c', func_listdir.format('benchmark'), check: true)
  benchmark_dirs = cmd_listdir_benchmark.stdout().strip().split('\n')

  foreach benchmark_dir : benchmark_dirs
    benchmark_filepath = join_paths('benchmark', benchmark_dir).replace('\\', '/')
    cmd_benchmark_src_c = run_command(
      'python', '-c', func_glob.format(benchmark_filepath, 'c', 'True'),
      check: true)
    benchmark_src_c = cmd_benchmark_src_c.stdout().strip().split('\n')
    benchmark_incdir = include_directories(benchmark_filepath, '.', 'gen')
    executable('benchmark_' + benchmark_dir, benchmark_src_c,
      include_directories: benchmark_incdir,
      link_with: libhaclog)
  endforeach
endif

################################
# gbenchmark
################################

if get_option('HACLOG_BUILD_GBENCHMARK')
  message('google benchmark without meson.build, ignore HACLOG_BUILD_GBENCHMARK')
endif
