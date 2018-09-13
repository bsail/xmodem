PROJECT_CEEDLING_ROOT = "vendor/ceedling"
load "#{PROJECT_CEEDLING_ROOT}/lib/ceedling.rb"
load "#{PROJECT_CEEDLING_ROOT}/lib/ceedling/rakefile.rb"

# Ceedling.load_project
ENV['GCOV_BUILD_PATH'] = "build/gcov/out"

task :default => %w[ test:all ]

desc "Generate LCOV test coverage report"
task :lcov => %w[ gcov:all ] do # 
  top_dir = `pwd`.chomp
  puts top_dir
  cmd = "rm -rf build/gcov/out/mock_* && rm -rf build/gcov/out/test_* && rm -rf build/gcov/out/cmock* && rm -rf build/gcov/out/unity* && mkdir -p build/lcov && lcov --capture --directory build/gcov/out --output-file coverage.info && genhtml -s -p #{top_dir} --legend coverage.info --output-directory build/lcov"
  puts cmd
  sh cmd
end

desc "Generate LCOV test coverage report (for Travis CI)"
task :travis => %w[ gcov:all ] do # 
  top_dir = `pwd`.chomp
  puts top_dir
  cmd = "rm -rf build/gcov/out/mock_* && rm -rf build/gcov/out/test_* && rm -rf build/gcov/out/cmock* && rm -rf build/gcov/out/unity* && mkdir -p build/lcov && lcov --capture --directory build/gcov/out --output-file coverage.info"
  puts cmd
  sh cmd
end
