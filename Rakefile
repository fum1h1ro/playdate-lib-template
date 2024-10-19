require 'rake/clean'

CURRENT_DIR = File.dirname(__FILE__)
PROJECT_NAME = File.basename(CURRENT_DIR).gsub(/[^a-zA-Z0-9-]/, '_')

SDK_ROOT = lambda do
  env = ENV['PLAYDATE_SDK_PATH']
  return env unless env.nil?

  File.read(File.expand_path('~/.Playdate/config')).each_line do |x|
    ssv = x.split(/\s+/)
    return ssv[1] if ssv[0] == 'SDKRoot'
  end
  raise 'cannot found SDK'
end.call
BUILD_DIR = '_build'.freeze
OUTPUT_DIR = '_output'.freeze

LUA_FILES = FileList['Source/**/*.lua']
LUALIB_DIR = 'lualib'.freeze
LUALIB_FILES = FileList["#{LUALIB_DIR}/**/*.lua"]
BUILD_TARGETS = ['Simulator', 'Device']
BUILD_TYPES = ['Debug', 'Release']
COMPILE_FLAGS_FILES = FileList['**/compile_flags.txt']

directory BUILD_DIR
directory OUTPUT_DIR

CLEAN.include(BUILD_DIR)
CLEAN.include(OUTPUT_DIR)
CLEAN.include('Source/pdex.*')
CLOBBER.include(COMPILE_FLAGS_FILES)

def all_targets_and_types(&block)
  BUILD_TARGETS.each do |target|
    BUILD_TYPES.each do |type|
      block.call(target.downcase, type.downcase)
    end
  end
end

def define_cmake_make_task(target, type, option)
  build_dir = "#{BUILD_DIR}/#{target}.#{type.downcase}"
  directory build_dir
  desc "Generate Makefile (#{target}, #{type.downcase})"
  task type.downcase => [build_dir] do |t|
    cd t.source do
      sh %(LIB_NAME="#{PROJECT_NAME}" cmake ../.. -DCMAKE_BUILD_TYPE=#{type} #{option}) unless File.exist?('Makefile')
    end
  end
end

def define_cmake_xcode_task(target, option)
  build_dir = "#{BUILD_DIR}/#{target}.xcode"
  directory build_dir
  desc "Generate Xcode project (#{target})"
  task target.downcase => build_dir do |t|
    cd t.source do
      sh %(LIB_NAME="#{PROJECT_NAME}" cmake ../.. #{option} -G Xcode)
      sh 'open .'
    end
  end
end

def define_build_task(target, type)
  build_dir = "#{BUILD_DIR}/#{target}.#{type.downcase}"
  desc "Build (#{target}, #{type.downcase})"
  task type.downcase => "cmake:#{target}:#{type.downcase}" do
    cd build_dir do
      FileList['*.dylib', '*.elf'].each do |binfile|
        rm_f binfile
      end
      sh %(PLAYDATE_LIB_PATH=#{File.expand_path("#{CURRENT_DIR}/#{LUALIB_DIR}")} make all)
    end
  end
end

def update_file(path)
  sh %(curl --create-dirs --silent --remote-time -o #{path} -H "Cache-Control: no-cache" https://raw.githubusercontent.com/fum1h1ro/playdate-lib-template/refs/heads/main/#{path})
end

task :default do
end

namespace :cmake do
  namespace :simulator do
    define_cmake_make_task('simulator', 'Debug', '')
    define_cmake_make_task('simulator', 'Release', '')
  end
  namespace :device do
    define_cmake_make_task('device', 'Debug', "-DCMAKE_TOOLCHAIN_FILE=#{SDK_ROOT}/C_API/buildsupport/arm.cmake")
    define_cmake_make_task('device', 'Release', "-DCMAKE_TOOLCHAIN_FILE=#{SDK_ROOT}/C_API/buildsupport/arm.cmake")
  end
  define_cmake_xcode_task('xcode', '')
end

desc 'Generate Makefile all'
task :cmake do
  all_targets_and_types do |target, type|
    sh "rake cmake:#{target}:#{type}"
  end
end

desc 'Build'
namespace :build do
  namespace :simulator do
    define_build_task('simulator', 'Debug')
    define_build_task('simulator', 'Release')
  end
  namespace :device do
    define_build_task('device', 'Debug')
    define_build_task('device', 'Release')
  end
end

desc 'Build all'
task :build do
  all_targets_and_types do |target, type|
    dir = "#{BUILD_DIR}/#{target}.#{type}"
    sh "rake build:#{target}:#{type}" if Dir.exist?(dir)
  end
end

desc 'Test'
task test: ['build:simulator:debug'] do
  cd "#{BUILD_DIR}/simulator.debug" do
    sh 'make test'
  rescue
    $stderr.puts "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
    $stderr.puts "FAILED\n"
    $stderr.puts "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
    print("================================================================================\n")
    File.read('Testing/Temporary/LastTest.log').each_line do |l|
      $stderr.print(l) if l =~ /FAILED/
    end
    print("================================================================================\n")
    raise
  end
end

desc 'Update'
task :update do
  [
    'CMakeLists.txt',
    'Rakefile',
    'compile_flags.txt.in',
    'acutest.h',
  ].each do |path|
    update_file(path)
  end
end
