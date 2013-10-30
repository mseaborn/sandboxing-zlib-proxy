
import re
import subprocess


test_file = '/home/mseaborn/fetched/gsdview.appspot.com/nativeclient-archive2/toolchain/6645/naclsdk_linux_arm-trusted.tgz'


def Main():
  cmds = [
      ('baseline', './out/decompress_timer'),
      ('proxy, without sandbox',
       'LD_PRELOAD=out/zlib_proxy_nosandbox.so ./out/decompress_timer'),
      ('proxy, with sandbox',
       'LD_PRELOAD=out/zlib_proxy.so ./out/decompress_timer'),
      ]
  baseline = None
  for name, cmd in cmds:
    proc = subprocess.Popen('%s %s' % (cmd, test_file),
                            shell=True, stdout=subprocess.PIPE)
    stdout = proc.communicate()[0]
    rc = proc.wait()
    assert rc == 0, (cmd, rc)
    parts = stdout.split()
    assert parts[0] == 'took'
    took = float(parts[1])
    if baseline is None:
      baseline = took
    print '%.3fs: %.3f%% slowdown: %s' % (
        took, (took / baseline - 1) * 100, name)


if __name__ == '__main__':
  Main()
