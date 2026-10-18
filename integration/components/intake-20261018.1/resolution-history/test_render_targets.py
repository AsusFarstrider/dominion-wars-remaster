import unittest
from analyze_render_targets import summarize

BIND = 'DWRT_BIND v=1 hr=0 rt=&1234 size=1600x1200 sub=0\n'
DRAW = ('DWRT_DRAW v=1 indexed=1 hr=0 rt=&1234 logical=800x600 actual=1600x1200 '
        'appvp=0,0,800,600 vp=0,0,1600,1200 tex0=256x256 tl=1 depth=1 type=4 primitives=120 sample=1 formats=23,22\n')


class RenderTargetTests(unittest.TestCase):
    def test_successful_matching_draw(self):
        self.assertTrue(summarize(BIND+DRAW)['verified_800x600_to_1600x1200_draw'])

    def test_bind_or_setting_alone_not_proof(self):
        self.assertFalse(summarize('ResolutionScale = app(2)\n'+BIND)['actual_render_dimensions_verified'])

    def test_draw_requires_preceding_successful_binding(self):
        for text in (DRAW+BIND, DRAW, BIND.replace('hr=0', 'hr=-1')+DRAW):
            self.assertFalse(summarize(text)['actual_render_dimensions_verified'])

    def test_failed_draw_not_proof(self):
        self.assertFalse(summarize(BIND+DRAW.replace('hr=0', 'hr=-1'))['actual_render_dimensions_verified'])

    def test_mismatched_binding_not_proof(self):
        self.assertFalse(summarize(BIND.replace('1600x1200', '800x600')+DRAW)['actual_render_dimensions_verified'])

    def test_overwritten_binding_not_proof(self):
        self.assertFalse(summarize(BIND+BIND.replace('hr=0', 'hr=-1')+DRAW)['actual_render_dimensions_verified'])

    def test_invalid_viewport_not_proof(self):
        self.assertFalse(summarize(BIND+DRAW.replace('vp=0,0,1600,1200', 'vp=0,0,1601,1200'))['actual_render_dimensions_verified'])

    def test_nonindexed_or_lines_do_not_qualify(self):
        for text in (DRAW.replace('indexed=1', 'indexed=0'), DRAW.replace('type=4', 'type=2')):
            self.assertFalse(summarize(BIND+text)['actual_render_dimensions_verified'])

    def test_baseline_can_verify_dimensions_without_doubling(self):
        result=summarize((BIND+DRAW).replace('1600x1200', '800x600').replace('vp=0,0,1600,1200', 'vp=0,0,800,600'))
        self.assertTrue(result['actual_render_dimensions_verified'])
        self.assertFalse(result['verified_800x600_to_1600x1200_draw'])

    def test_malformed_does_not_verify(self):
        report=summarize('DWRT_DRAW v=1 hr=0\n')
        self.assertTrue(report['malformed'])
        self.assertFalse(report['actual_render_dimensions_verified'])

    def test_fractional_1440_target(self):
        report=summarize((BIND+DRAW).replace('1600x1200', '1920x1440').replace('vp=0,0,1600,1200','vp=0,0,1920,1440'))
        self.assertTrue(report['actual_render_dimensions_verified'])
        self.assertEqual(report['observed_verified_target_sizes'], [(1920,1440)])
        self.assertFalse(report['verified_800x600_to_1600x1200_draw'])


if __name__ == '__main__':
    unittest.main()
