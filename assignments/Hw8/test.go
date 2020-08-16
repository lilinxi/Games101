func Test_Out(t *testing.T) {
	a := 2.0
	y0 := math.Exp(-a * 0)
	di := 2.0

	y_1 := y0
	y_1_0 := y0

	y_2 := y0
	y_2_0 := y0

	for i := di; i < 100; i += di {
		y := math.Exp(-a * i)

		y_1 = y_1 - a*di*y_1_0
		y_1_0 = y_1

		y_2 = y_2_0 / (1 + a*di)
		y_2_0 = y_2

		log.Printf("y: %v, %v, %v", y, y_1, y_2)
		//log.Printf("residue:  %v, %v", math.Abs(y-y_1), math.Abs(y-y_2))
	}
}
