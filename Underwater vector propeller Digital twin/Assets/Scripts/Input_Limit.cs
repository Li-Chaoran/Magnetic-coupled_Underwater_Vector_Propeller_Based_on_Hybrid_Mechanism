using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Input_Limit : MonoBehaviour
{
    public GameObject magnetic_coupling_show;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
    public void OnInputValueChanged(string value)
    {
        // Debug.Log("输入框内容改变：" + value);                   //调试信息
        magnetic_coupling_show.GetComponent<Magnetic_coupling>().limit_pwmrange = int.Parse(value);
    }
}
